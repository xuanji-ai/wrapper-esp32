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
    Logger& logger_;
    nvs_handle_t nvs_handle_;
public:
    Nvs(Logger& logger);
    ~Nvs();
    bool Init();
    bool Erase();
    bool Commit();

    bool OpenNamespace(std::string_view namespace_name, nvs_open_mode_t open_mode);
    bool EraseKey(std::string_view key);
    
    template<typename T>
    bool SetValue(std::string_view key, T value);
    template<typename T>
    bool GetValue(std::string_view key, T& out_value);
    
    bool SetString(std::string_view key, std::string value);
    bool GetString(std::string_view key, std::string& out_value);
};

class Event
{
    Logger& logger_;
    esp_event_loop_handle_t loop_handle_ = nullptr;
    bool is_default_loop_ = false;

public:
    Event(Logger& logger);
    ~Event();

    // Loop Management
    bool CreateLoopDefault();
    bool DeleteLoopDefault();
    
    bool CreateLoop(const esp_event_loop_args_t& args);
    bool DeleteLoop();
    
    bool RunLoop(TickType_t ticks_to_run);

    // Handler Management
    bool Register(esp_event_base_t event_base, int32_t event_id, esp_event_handler_t event_handler, void* event_handler_arg, esp_event_handler_instance_t* instance);
    bool Unregister(esp_event_base_t event_base, int32_t event_id, esp_event_handler_instance_t instance);
    
    // Post Events
    bool Post(esp_event_base_t event_base, int32_t event_id, const void* event_data, size_t event_data_size, TickType_t ticks_to_wait);
    
    // ISR Post
    bool PostFromIsr(esp_event_base_t event_base, int32_t event_id, const void* event_data, size_t event_data_size, BaseType_t* task_unblocked);

    // Diagnostics
    bool Dump(FILE* file);
};

} // namespace wrapper
