I will implement the `Nvs` class in `example/main/app_main.cpp` by wrapping the ESP-IDF NVS API.

**Research Findings:**
- The file `example/main/app_main.cpp` currently contains a skeleton `Nvs` class.
- The standard `<nvs.h>` and `<nvs_flash.h>` headers provide the C API for NVS operations (`nvs_open`, `nvs_set_*`, `nvs_get_*`, etc.).

**Implementation Plan:**
1.  **Modify `class Nvs` in `app_main.cpp`**:
    -   **Member Variables**: `nvs_handle_t m_handle` to manage the NVS handle. `Logger& m_logger` for logging.
    -   **Constructor/Destructor**:
        -   Initialize `m_handle` to 0.
        -   In Destructor, call `nvs_close()` if the handle is valid.
    -   **Init()**: Call `nvs_flash_init()`. Implement the standard recovery logic: if `ESP_ERR_NVS_NO_FREE_PAGES` or `ESP_ERR_NVS_NEW_VERSION_FOUND` occurs, call `nvs_flash_erase()` and then `nvs_flash_init()` again.
    -   **Open()**: Wrap `nvs_open`. Accept `std::string` for the namespace name.
    -   **Commit()**: Wrap `nvs_commit`.
    -   **Set/Get Methods**:
        -   Implement generic or overloaded methods for `int32_t` and `std::string`.
        -   Use `std::string` for string keys and values.
        -   Use references for output parameters.
        -   Return `esp_err_t` for all operations.
        -   Add error logging using the `m_logger` member.

**Code Structure Preview:**
```cpp
class Nvs {
    // ... members ...
public:
    // ... Init, Erase ...
    esp_err_t Open(const std::string& name, nvs_open_mode_t mode = NVS_READWRITE);
    esp_err_t SetI32(const std::string& key, int32_t value);
    esp_err_t GetI32(const std::string& key, int32_t& out_value);
    esp_err_t SetString(const std::string& key, const std::string& value);
    esp_err_t GetString(const std::string& key, std::string& out_value);
    // ... Commit ...
};
```
I will edit the file to replace the existing class definition with this complete implementation.