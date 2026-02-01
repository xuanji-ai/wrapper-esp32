I will implement the `WifiProvider` class to support Station, SoftAP, SmartConfig, and Provisioning modes.

**Plan:**

1.  **Update `src/wrapper/wifi.hpp`**:
    *   Add necessary private members to `WifiProvider` class:
        *   `esp_netif_t* m_netif` for network interface handle.
        *   `void* m_event_group` (or `EventGroupHandle_t`) for synchronization in `Wait()`.
        *   Static event handler wrapper to interface with ESP-IDF C-style callbacks.

2.  **Create `src/wrapper/wifi.cpp`**:
    *   Implement `StartStationMode`: Initialize WiFi in Station mode, configure SSID/Password, and connect.
    *   Implement `StartSoftApMode`: Initialize WiFi in SoftAP mode, configure SSID/Password.
    *   Implement `StartSmartConfigMode`: Use `esp_smartconfig` to listen for EspTouch configuration.
    *   Implement `StartEasyConnectMode`: Use `wifi_provisioning` component (Unified Provisioning) to allow configuration via BLE or SoftAP. *Note: I will use the `wifi_provisioning` manager as referenced in the documentation.*
    *   Implement `Wait`: Block until WiFi is connected (got IP) or provisioning is done.
    *   Implement `Stop`: De-initialize and stop WiFi/Provisioning.

3.  **Dependencies**:
    *   Ensure proper inclusion of `esp_wifi.h`, `esp_event.h`, `esp_smartconfig.h`, and `wifi_provisioning/manager.h`.
