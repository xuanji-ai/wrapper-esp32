#pragma once

#include <string>
#include <string_view>
#include <esp_event.h>
#include <esp_wifi.h>
#include <esp_netif.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include "wrapper/logger.hpp"
#include "wrapper/soc.hpp"

namespace wrapper
{

class WifiProvider
{
    Logger& m_logger;
    esp_netif_t* m_netif = nullptr;
    EventGroupHandle_t m_event_group = nullptr;
    
    static void EventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

public:
    WifiProvider(Logger& logger);
    ~WifiProvider();
    bool StartStationMode(std::string_view ssid, std::string_view password);  
    bool StartSoftApMode(std::string_view ssid, std::string_view password, int max_connections = 4);
    bool StartSmartConfigMode();
    bool StartEasyConnectMode();
    bool Wait();
    bool Stop();
};

}// namespace wrapper
