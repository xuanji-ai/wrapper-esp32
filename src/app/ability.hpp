#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/event_groups.h>
#include <freertos/timers.h>
#include <freertos/queue.h>

#include "wrapper/logger.hpp"

namespace monncake
{

enum class AbilityType
{
  None,
  Base,
  Basic,
  Worker,
  Service,
  Ui,
  App,
  Custom,
};

enum class AbilityPriority : UBaseType_t
{
  Low = 1,
  Normal = 5,
  High = 10,
};

class AbilityBase
{

protected:
  std::string_view name_{"AbilityBase"};
  AbilityType type_{AbilityType::None};
  int id_{-1};

  std::function<void(void*)> task_function_ {[](void*) {}};
  const configSTACK_DEPTH_TYPE stack_size_ {2048};
  void* task_parameter_{nullptr};
  UBaseType_t priority_{AbilityPriority::Normal};
  TaskHandle_t task_handle_{nullptr};
public:

  void SetName(const std::string_view& name) { return name_ = name; }
  std::string_view GetName() const { return name_; }

  void SetType(AbilityType type) { return type_ = type; }
  AbilityType GetType() const { return type_; }

  void SetId(int id) { return id_ = id; }
  int GetId() const { return id_; }

  void SetTaskFunction(std::function<void(void*)> func) { task_function_ = func; }
  std::function<void(void*)> GetTaskFunction() const { return task_function_; }

  void SetStackSize(configSTACK_DEPTH_TYPE stack_size) { return stack_size_ = stack_size; }
  configSTACK_DEPTH_TYPE GetStackSize() const { return stack_size_; }

  void SetTaskParameter(void* parameter) { task_parameter_ = parameter; }
  void* GetTaskParameter() const { return task_parameter_; }

  void SetPriority(AbilityPriority priority) { priority_ = static_cast<UBaseType_t>(priority); }
  AbilityPriority GetPriority() const { return static_cast<AbilityPriority>(priority_); }

  TaskHandle_t GetTaskHandle() const { return task_handle_; }

  AbilityBase() = default;
  virtual ~AbilityBase() = default;

  virtual void Create()
  {

  }
  
  virtual void Run()
  {
    xTaskCreate(task_function_.target<void(*)(void*)>(), name_.data(), stack_size_, task_parameter_, priority_, &task_handle_);
  }

  virtual void Destroy()
  {
    if (task_handle_ != nullptr)
    {
      vTaskDelete(task_handle_);
      task_handle_ = nullptr;
    }
  }
}