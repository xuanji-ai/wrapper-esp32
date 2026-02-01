I will implement the `Nvs` class in `example/main/app_main.cpp` to provide a functional wrapper around the ESP-IDF NVS (Non-Volatile Storage) API, as requested.

The implementation will include:
1.  **Constructor & Destructor**: Initialize members and ensure `nvs_close` is called to prevent resource leaks.
2.  **Open Method**: To open a namespace in NVS (`nvs_open`).
3.  **Commit Method**: To save changes to flash (`nvs_commit`).
4.  **Data Access Methods**: 
    - `SetI32` / `GetI32` for integer storage.
    - `SetString` / `GetString` for string storage.
5.  **Logging**: Use the `m_logger` member to log errors during NVS operations.

**Proposed Class Structure:**
```cpp
class Nvs
{
  Logger& m_logger;
  nvs_handle_t m_nvs_handle;

public:
    Nvs(Logger& logger) : m_logger(logger), m_nvs_handle(0) {}

    ~Nvs() {
        if (m_nvs_handle) {
            nvs_close(m_nvs_handle);
        }
    }

    esp_err_t Init() {
        esp_err_t err = nvs_flash_init();
        if (err != ESP_OK) {
             m_logger.Error("NVS Flash Init failed: %s", esp_err_to_name(err));
        }
        return err;
    }

    esp_err_t Erase() {
        return nvs_flash_erase();
    }

    esp_err_t Open(const char* namespace_name, nvs_open_mode_t open_mode = NVS_READWRITE) {
        esp_err_t err = nvs_open(namespace_name, open_mode, &m_nvs_handle);
        if (err != ESP_OK) {
            m_logger.Error("NVS Open failed: %s", esp_err_to_name(err));
        }
        return err;
    }

    esp_err_t Commit() {
        return nvs_commit(m_nvs_handle);
    }
    
    // ... Set/Get methods for int32 and string ...
};
```
I will edit `example/main/app_main.cpp` to replace the existing skeleton with this full implementation.