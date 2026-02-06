#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/event_groups.h>

#include <string>
#include <functional>

namespace wrapper
{

  inline void Delay(uint32_t milliseconds)
  {
    vTaskDelay(pdMS_TO_TICKS(milliseconds));
  }

  inline void DelayTicks(uint32_t ticks)
  {
    vTaskDelay(ticks);
  }

  inline void DelayUntil(uint32_t* previous_wake_time, uint32_t time_increment_ms)
  {
    vTaskDelayUntil(previous_wake_time, pdMS_TO_TICKS(time_increment_ms));
  }

  inline void DelayUntilTicks(uint32_t* previous_wake_time, uint32_t time_increment_ticks)
  {
    vTaskDelayUntil(previous_wake_time, time_increment_ticks);
  }

class Task
{
  std::string name_;
  std::function<void(void*)> function_;
  void* arg_;
  configSTACK_DEPTH_TYPE stack_depth_;
  UBaseType_t priority_;
  TaskHandle_t handle_;

public:

  static constexpr uint32_t MAX_DELAY_TICK = portMAX_DELAY;
  static constexpr uint32_t MAX_DELAY_MS   = pdTICKS_TO_MS(portMAX_DELAY);

  Task(const std::string& name,
       std::function<void(void*)> function,
       void* arg,
       uint32_t stack_depth,
       UBaseType_t priority)
    : name_(name),
      function_(function),
      arg_(arg),
      stack_depth_(stack_depth),
      priority_(priority),
      handle_(nullptr)
  {
  }

  ~Task()
  {
  }

  bool Create()
  {
    BaseType_t result = xTaskCreate(
      [](void* pvParameters) {
        Task* task = static_cast<Task*>(pvParameters);
        task->function_(task->arg_);
        vTaskDelete(nullptr);
      },
      name_.c_str(),
      stack_depth_,
      this,
      priority_,
      &handle_);
    return result == pdPASS;
  }

  void Delete()
  {
    if (handle_ != nullptr)
    {
      vTaskDelete(handle_);
      handle_ = nullptr;
    }
  }

  void Suspend()
  {
    if (handle_ != nullptr)
    {
      vTaskSuspend(handle_);
    }
  }

  void Resume()
  {
    if (handle_ != nullptr)
    {
      vTaskResume(handle_);
    }
  }

  void SetPriority(UBaseType_t new_priority)
  {
    if (handle_ != nullptr)
    {
      vTaskPrioritySet(handle_, new_priority);
      priority_ = new_priority;
    }
  }

  std::string GetName() const
  {
    return name_;
  }

  uint32_t GetStackDepth() const
  {
    return stack_depth_;
  }

  UBaseType_t GetPriority() const
  {
    return priority_;
  }

  TaskHandle_t GetHandle() const
  {
    return handle_;
  }

};

template <typename T>
class Queue
{
  QueueHandle_t handle_;
  UBaseType_t length_;

public:
  Queue(UBaseType_t length) : length_(length), handle_(nullptr)
  {
    handle_ = xQueueCreate(length, sizeof(T));
  }

  ~Queue()
  {
    if (handle_ != nullptr)
    {
      vQueueDelete(handle_);
      handle_ = nullptr;
    }
  }

  bool Send(const T& item, TickType_t wait_ticks = portMAX_DELAY)
  {
    if (handle_ == nullptr)
      return false;
    return xQueueSend(handle_, &item, wait_ticks) == pdTRUE;
  }

  bool SendFromISR(const T& item, BaseType_t* pxHigherPriorityTaskWoken)
  {
    if (handle_ == nullptr)
      return false;
    return xQueueSendFromISR(handle_, &item, pxHigherPriorityTaskWoken) == pdTRUE;
  }

  bool SendToFront(const T& item, TickType_t wait_ticks = portMAX_DELAY)
  {
    if (handle_ == nullptr)
      return false;
    return xQueueSendToFront(handle_, &item, wait_ticks) == pdTRUE;
  }

  bool SendToFrontFromISR(const T& item, BaseType_t* pxHigherPriorityTaskWoken)
  {
    if (handle_ == nullptr)
      return false;
    return xQueueSendToFrontFromISR(handle_, &item, pxHigherPriorityTaskWoken) == pdTRUE;
  }

  bool Overwrite(const T& item)
  {
    if (handle_ == nullptr)
      return false;
    return xQueueOverwrite(handle_, &item) == pdTRUE;
  }

  bool OverwriteFromISR(const T& item, BaseType_t* pxHigherPriorityTaskWoken)
  {
    if (handle_ == nullptr)
      return false;
    return xQueueOverwriteFromISR(handle_, &item, pxHigherPriorityTaskWoken) == pdTRUE;
  }

  bool Receive(T& item, TickType_t wait_ticks = portMAX_DELAY)
  {
    if (handle_ == nullptr)
      return false;
    return xQueueReceive(handle_, &item, wait_ticks) == pdTRUE;
  }

  bool ReceiveFromISR(T& item, BaseType_t* pxHigherPriorityTaskWoken)
  {
    if (handle_ == nullptr)
      return false;
    return xQueueReceiveFromISR(handle_, &item, pxHigherPriorityTaskWoken) == pdTRUE;
  }

  bool Peek(T& item, TickType_t wait_ticks = portMAX_DELAY)
  {
    if (handle_ == nullptr)
      return false;
    return xQueuePeek(handle_, &item, wait_ticks) == pdTRUE;
  }

  bool PeekFromISR(T& item)
  {
    if (handle_ == nullptr)
      return false;
    return xQueuePeekFromISR(handle_, &item) == pdTRUE;
  }

  UBaseType_t MessagesWaiting() const
  {
    if (handle_ == nullptr)
      return 0;
    return uxQueueMessagesWaiting(handle_);
  }

  UBaseType_t MessagesWaitingFromISR() const
  {
    if (handle_ == nullptr)
      return 0;
    return uxQueueMessagesWaitingFromISR(handle_);
  }

  UBaseType_t SpacesAvailable() const
  {
    if (handle_ == nullptr)
      return 0;
    return uxQueueSpacesAvailable(handle_);
  }

  bool IsQueueEmptyFromISR() const
  {
    if (handle_ == nullptr)
      return true; // Treat null as empty or error
    return xQueueIsQueueEmptyFromISR(handle_) == pdTRUE;
  }

  bool IsQueueFullFromISR() const
  {
    if (handle_ == nullptr)
      return false; // Treat null as not full or error
    return xQueueIsQueueFullFromISR(handle_) == pdTRUE;
  }

  void Reset()
  {
    if (handle_ != nullptr)
    {
      xQueueReset(handle_);
    }
  }

  QueueHandle_t GetHandle() const
  {
    return handle_;
  }

  bool IsValid() const
  {
    return handle_ != nullptr;
  }
};

class Semaphore
{
protected:
  SemaphoreHandle_t handle_;

public:
  Semaphore() : handle_(nullptr) {}

  virtual ~Semaphore()
  {
    if (handle_ != nullptr)
    {
      vSemaphoreDelete(handle_);
      handle_ = nullptr;
    }
  }

  bool Take(TickType_t wait_ticks = portMAX_DELAY)
  {
    if (handle_ == nullptr) return false;
    return xSemaphoreTake(handle_, wait_ticks) == pdTRUE;
  }

  bool Give()
  {
    if (handle_ == nullptr) return false;
    return xSemaphoreGive(handle_) == pdTRUE;
  }

  bool TakeFromISR(BaseType_t* pxHigherPriorityTaskWoken)
  {
    if (handle_ == nullptr) return false;
    return xSemaphoreTakeFromISR(handle_, pxHigherPriorityTaskWoken) == pdTRUE;
  }

  bool GiveFromISR(BaseType_t* pxHigherPriorityTaskWoken)
  {
    if (handle_ == nullptr) return false;
    return xSemaphoreGiveFromISR(handle_, pxHigherPriorityTaskWoken) == pdTRUE;
  }

  SemaphoreHandle_t GetHandle() const
  {
    return handle_;
  }

  bool IsValid() const
  {
    return handle_ != nullptr;
  }
};

class BinarySemaphore : public Semaphore
{
public:
  BinarySemaphore()
  {
    handle_ = xSemaphoreCreateBinary();
  }
};

class CountingSemaphore : public Semaphore
{
public:
  CountingSemaphore(UBaseType_t max_count, UBaseType_t initial_count)
  {
    handle_ = xSemaphoreCreateCounting(max_count, initial_count);
  }
};

class Mutex : public Semaphore
{
public:
  Mutex()
  {
    handle_ = xSemaphoreCreateMutex();
  }
};

class RecursiveMutex : public Semaphore
{
public:
  RecursiveMutex()
  {
    handle_ = xSemaphoreCreateRecursiveMutex();
  }

  bool Take(TickType_t wait_ticks = portMAX_DELAY)
  {
    if (handle_ == nullptr) return false;
    return xSemaphoreTakeRecursive(handle_, wait_ticks) == pdTRUE;
  }

  bool Give()
  {
    if (handle_ == nullptr) return false;
    return xSemaphoreGiveRecursive(handle_) == pdTRUE;
  }
};

class EventGroup
{
  EventGroupHandle_t handle_;

public:
  EventGroup()
  {
    handle_ = xEventGroupCreate();
  }

  ~EventGroup()
  {
    if (handle_ != nullptr)
    {
      vEventGroupDelete(handle_);
      handle_ = nullptr;
    }
  }

  EventBits_t WaitBits(const EventBits_t bits_to_wait_for,
                       const bool clear_on_exit,
                       const bool wait_for_all_bits,
                       TickType_t wait_ticks = portMAX_DELAY)
  {
    if (handle_ == nullptr) return 0;
    return xEventGroupWaitBits(handle_, bits_to_wait_for, clear_on_exit, wait_for_all_bits, wait_ticks);
  }

  EventBits_t SetBits(const EventBits_t bits_to_set)
  {
    if (handle_ == nullptr) return 0;
    return xEventGroupSetBits(handle_, bits_to_set);
  }

  EventBits_t SetBitsFromISR(const EventBits_t bits_to_set, BaseType_t* pxHigherPriorityTaskWoken)
  {
    if (handle_ == nullptr) return 0;
    return xEventGroupSetBitsFromISR(handle_, bits_to_set, pxHigherPriorityTaskWoken);
  }

  EventBits_t ClearBits(const EventBits_t bits_to_clear)
  {
    if (handle_ == nullptr) return 0;
    return xEventGroupClearBits(handle_, bits_to_clear);
  }

  EventBits_t ClearBitsFromISR(const EventBits_t bits_to_clear)
  {
    if (handle_ == nullptr) return 0;
    return xEventGroupClearBitsFromISR(handle_, bits_to_clear);
  }

  EventBits_t GetBits() const
  {
    if (handle_ == nullptr) return 0;
    return xEventGroupGetBits(handle_);
  }

  EventBits_t GetBitsFromISR() const
  {
    if (handle_ == nullptr) return 0;
    return xEventGroupGetBitsFromISR(handle_);
  }

  EventBits_t Sync(const EventBits_t bits_to_set,
                   const EventBits_t bits_to_wait_for,
                   TickType_t wait_ticks = portMAX_DELAY)
  {
    if (handle_ == nullptr) return 0;
    return xEventGroupSync(handle_, bits_to_set, bits_to_wait_for, wait_ticks);
  }

  EventGroupHandle_t GetHandle() const
  {
    return handle_;
  }

  bool IsValid() const
  {
    return handle_ != nullptr;
  }
};

}  // namespace wrapper