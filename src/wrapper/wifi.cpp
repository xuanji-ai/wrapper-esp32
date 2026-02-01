#include "wrapper/wifi.hpp"
#include <cstring>
#include <memory>
#include <esp_smartconfig.h>
#include <wifi_provisioning/manager.h>
#include <wifi_provisioning/scheme_ble.h>

static const int WIFI_CONNECTED_BIT = BIT0;
static const int WIFI_FAIL_BIT      = BIT1;
static const int WIFI_PROV_DONE_BIT = BIT2;

namespace wrapper
{

WifiProvider::WifiProvider(Logger& logger) : m_logger(logger)
{
    esp_netif_init(); // Initialize the underlying TCP/IP stack
    m_event_group = xEventGroupCreate();
    esp_event_loop_create_default(); // Ensure default loop exists
}

WifiProvider::~WifiProvider()
{
    Stop();
    if (m_event_group) {
        vEventGroupDelete(m_event_group);
    }
}

void WifiProvider::EventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    WifiProvider* self = static_cast<WifiProvider*>(arg);

    if (event_base == WIFI_EVENT) {
        if (event_id == WIFI_EVENT_STA_START) {
            esp_wifi_connect();
        } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
            self->m_logger.Warning("WiFi Disconnected, retrying...");
            esp_wifi_connect();
            xEventGroupClearBits(self->m_event_group, WIFI_CONNECTED_BIT);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        self->m_logger.Info("Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(self->m_event_group, WIFI_CONNECTED_BIT);
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        
        // Allocate wifi_config on heap to avoid stack overflow in system event task
        std::unique_ptr<wifi_config_t> wifi_config(new wifi_config_t());
        memset(wifi_config.get(), 0, sizeof(wifi_config_t));

        memcpy(wifi_config->sta.ssid, evt->ssid, sizeof(wifi_config->sta.ssid));
        memcpy(wifi_config->sta.password, evt->password, sizeof(wifi_config->sta.password));
        wifi_config->sta.bssid_set = evt->bssid_set;
        if (wifi_config->sta.bssid_set == true) {
            memcpy(wifi_config->sta.bssid, evt->bssid, sizeof(wifi_config->sta.bssid));
        }

        self->m_logger.Info("SmartConfig: Got SSID: %s", evt->ssid);
        esp_wifi_disconnect();
        esp_wifi_set_config(WIFI_IF_STA, wifi_config.get());
        esp_wifi_connect();
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) {
        xEventGroupSetBits(self->m_event_group, WIFI_PROV_DONE_BIT);
    } else if (event_base == WIFI_PROV_EVENT) {
        if (event_id == WIFI_PROV_START) {
            self->m_logger.Info("Provisioning started");
        } else if (event_id == WIFI_PROV_CRED_RECV) {
            wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
            self->m_logger.Info("Received Wi-Fi credentials: SSID: %s", (const char *) wifi_sta_cfg->ssid);
        } else if (event_id == WIFI_PROV_CRED_FAIL) {
            wifi_prov_sta_fail_reason_t *reason = (wifi_prov_sta_fail_reason_t *)event_data;
            self->m_logger.Error("Provisioning failed: %s", 
                (*reason == WIFI_PROV_STA_AUTH_ERROR) ? "Auth Error" : "AP Not Found");
        } else if (event_id == WIFI_PROV_CRED_SUCCESS) {
            self->m_logger.Info("Provisioning successful");
        } else if (event_id == WIFI_PROV_END) {
            self->m_logger.Info("Provisioning ended");
            wifi_prov_mgr_deinit();
            xEventGroupSetBits(self->m_event_group, WIFI_PROV_DONE_BIT);
        }
    }
}

bool WifiProvider::StartStationMode(std::string_view ssid, std::string_view password)
{
    m_netif = esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &WifiProvider::EventHandler, this);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &WifiProvider::EventHandler, this);

    wifi_config_t wifi_config = {};
    std::memcpy(wifi_config.sta.ssid, ssid.data(), std::min(ssid.size(), sizeof(wifi_config.sta.ssid)));
    std::memcpy(wifi_config.sta.password, password.data(), std::min(password.size(), sizeof(wifi_config.sta.password)));
    
    wifi_config.sta.threshold.authmode = password.empty() ? WIFI_AUTH_OPEN : WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    m_logger.Info("Started Station Mode, SSID: %s", ssid.data());
    return true;
}

bool WifiProvider::StartSoftApMode(std::string_view ssid, std::string_view password, int max_connections)
{
    m_netif = esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &WifiProvider::EventHandler, this);

    wifi_config_t wifi_config = {};
    std::memcpy(wifi_config.ap.ssid, ssid.data(), std::min(ssid.size(), sizeof(wifi_config.ap.ssid)));
    std::memcpy(wifi_config.ap.password, password.data(), std::min(password.size(), sizeof(wifi_config.ap.password)));
    
    wifi_config.ap.ssid_len = ssid.size();
    wifi_config.ap.channel = 1;
    wifi_config.ap.max_connection = max_connections;
    wifi_config.ap.authmode = password.empty() ? WIFI_AUTH_OPEN : WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    m_logger.Info("Started SoftAP Mode, SSID: %s", ssid.data());
    return true;
}

bool WifiProvider::StartSmartConfigMode()
{
    m_netif = esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &WifiProvider::EventHandler, this);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &WifiProvider::EventHandler, this);
    esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &WifiProvider::EventHandler, this);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    esp_smartconfig_set_type(SC_TYPE_ESPTOUCH);
    smartconfig_start_config_t cfg_sc = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_smartconfig_start(&cfg_sc));

    m_logger.Info("Started SmartConfig Mode");
    return true;
}

bool WifiProvider::StartEasyConnectMode()
{
    // Initialize networking
    m_netif = esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &WifiProvider::EventHandler, this);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &WifiProvider::EventHandler, this);
    esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &WifiProvider::EventHandler, this);

    wifi_prov_mgr_config_t config = {
        .scheme = wifi_prov_scheme_ble,
        .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM
    };
    ESP_ERROR_CHECK(wifi_prov_mgr_init(config));

    bool provisioned = false;
    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));

    if (!provisioned) {
        m_logger.Info("Starting provisioning");
        // We use "PROV_ESP32" as service name and NULL (no security) or simple security
        // For simplicity using WIFI_PROV_SECURITY_1 with NULL PoP (proof of possession)
        ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(WIFI_PROV_SECURITY_1, NULL, "PROV_ESP32", NULL));
    } else {
        m_logger.Info("Already provisioned, starting WiFi");
        wifi_prov_mgr_deinit();
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_start());
    }

    return true;
}

bool WifiProvider::Wait()
{
    EventBits_t bits = xEventGroupWaitBits(m_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        return true;
    } else if (bits & WIFI_FAIL_BIT) {
        return false;
    }
    return false;
}

bool WifiProvider::Stop()
{
    esp_wifi_stop();
    esp_wifi_deinit();
    
    esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &WifiProvider::EventHandler);
    esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &WifiProvider::EventHandler);
    esp_event_handler_unregister(SC_EVENT, ESP_EVENT_ANY_ID, &WifiProvider::EventHandler);
    esp_event_handler_unregister(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &WifiProvider::EventHandler);

    if (m_netif) {
        esp_netif_destroy(m_netif);
        m_netif = nullptr;
    }
    return true;
}

} // namespace wrapper
