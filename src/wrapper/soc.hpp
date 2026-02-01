#pragma once
#include <nvs_flash.h>
#include <esp_event.h>
#include <esp_wifi.h>
#include <string>
#include <string_view>
#include "wrapper/logger.hpp"

namespace wrapper
{

class Nvs
{
    Logger& m_logger;
    nvs_handle_t m_nvs_handle;
public:
    Nvs(Logger& logger);
    ~Nvs();
    esp_err_t Init();
    esp_err_t Erase();
    esp_err_t Commit();

    esp_err_t OpenNamespace(std::string_view namespace_name, nvs_open_mode_t open_mode);
    esp_err_t EraseKey(std::string_view key);
    
    template<typename T>
    esp_err_t SetValue(std::string_view key, T value);
    template<typename T>
    esp_err_t GetValue(std::string_view key, T& out_value);
    
    esp_err_t SetString(std::string_view key, std::string value);
    esp_err_t GetString(std::string_view key, std::string& out_value);
};

class Event
{
    Logger& m_logger;
    esp_event_loop_handle_t m_loop_handle = nullptr;
    bool m_is_default_loop = false;

public:
    Event(Logger& logger);
    ~Event();

    // Loop Management
    esp_err_t CreateLoopDefault();
    esp_err_t DeleteLoopDefault();
    
    esp_err_t CreateLoop(const esp_event_loop_args_t& args);
    esp_err_t DeleteLoop();
    
    esp_err_t RunLoop(TickType_t ticks_to_run);

    // Handler Management
    esp_err_t Register(esp_event_base_t event_base, int32_t event_id, esp_event_handler_t event_handler, void* event_handler_arg, esp_event_handler_instance_t* instance);
    esp_err_t Unregister(esp_event_base_t event_base, int32_t event_id, esp_event_handler_instance_t instance);
    
    // Post Events
    esp_err_t Post(esp_event_base_t event_base, int32_t event_id, const void* event_data, size_t event_data_size, TickType_t ticks_to_wait);
    
    // ISR Post
    esp_err_t PostFromIsr(esp_event_base_t event_base, int32_t event_id, const void* event_data, size_t event_data_size, BaseType_t* task_unblocked);

    // Diagnostics
    esp_err_t Dump(FILE* file);
};

} // namespace wrapper
