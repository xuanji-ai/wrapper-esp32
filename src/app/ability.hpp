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
//!--------------------------------------------------------------------------------------------------------------------
  enum class AbilityType : UBaseType_t
  {
    None = 0,
    Polling,
    Queue,
    Task,
    Service,
    Ui,
    Max
  };
//!--------------------------------------------------------------------------------------------------------------------
  // enum class AbilityPriority : UBaseType_t
  // {
  //   Async = 0,
  //   Sync = Async + static_cast<UBaseType_t>(AbilityType::Max)
  // };
//!--------------------------------------------------------------------------------------------------------------------
  class Ability
  {
  public:
    AbilityType type_{AbilityType::None};
    int id_{-1};
    std::string_view name_{"Ability"};

    void SetType(AbilityType type) { type_ = type; }
    AbilityType GetType() const { return type_; }

    void SetId(int id) { id_ = id; }
    int GetId() const { return id_; }

    void SetName(const std::string_view &name) { name_ = name; }
    std::string_view GetName() const { return name_; }

    Ability(AbilityType type, int id, const std::string_view &name)
        : type_(type), id_(id), name_(name)
    {
    }
    virtual ~Ability() = default;
  };
//!--------------------------------------------------------------------------------------------------------------------
  // 由AbilityManager轮询调用
  class PollingAbility : public Ability
  {
  public:
    PollingAbility(AbilityType type, int id, const std::string_view &name) : Ability(type, id, name)
    {
    }
    virtual ~PollingAbility() = default;
    virtual void Update() = 0;
  };
//!--------------------------------------------------------------------------------------------------------------------
  // 由FreeRTOS任务调度器调度运行
  class TaskAbility : public Ability
  {
  public:
    enum class State
    {
      Running,
      Suspended,
      Stopped,
    };

    State state_{State::Stopped};

    bool IsRunning() const { return state_ == State::Running; }
    bool IsSuspended() const { return state_ == State::Suspended; }
    bool IsStopped() const { return state_ == State::Stopped; }

    configSTACK_DEPTH_TYPE stack_size_;
    void *task_parameter_;
    UBaseType_t priority_;
    TaskHandle_t task_handle_;

    void SetState(State state) { state_ = state; }
    State GetState() const { return state_; }

    void SetStackSize(int stack_size) { stack_size_ = static_cast<configSTACK_DEPTH_TYPE>(stack_size); }
    int GetStackSize() const { return static_cast<int>(stack_size_); }

    void SetTaskParameter(void *parameter) { task_parameter_ = parameter; }
    void *GetTaskParameter() const { return task_parameter_; }

    void SetPriority(int priority) { priority_ = static_cast<UBaseType_t>(priority); }
    int GetPriority() const { return priority_; }

    TaskHandle_t GetTaskHandle() const { return task_handle_; }

    TaskAbility(AbilityType type, int id, const std::string_view &name) : Ability(type, id, name)
    {
      state_ = State::Stopped;
      stack_size_ = 2048;
      task_parameter_ = nullptr;
      priority_ = 0;
      task_handle_ = nullptr;
    }

    virtual ~TaskAbility() = default;

    virtual void Start()
    {
      if (task_handle_ != nullptr)
      {
        UBaseType_t result = xTaskCreate(
            [](void *param)
            {
              TaskAbility *ability = static_cast<TaskAbility *>(param);
              ability->TaskFunc();
              ability->SetState(State::Stopped);
              vTaskDelete(nullptr);
            },
            name_.data(), stack_size_, this, priority_, &task_handle_);

        if (result == pdPASS)
        {
          state_ = State::Running;
        }
      }
    }

    virtual void Suspend()
    {
      if (task_handle_ != nullptr)
      {
        vTaskSuspend(task_handle_);
        state_ = State::Suspended;
      }
    }

    virtual void Resume()
    {
      if (task_handle_ != nullptr)
      {
        vTaskResume(task_handle_);
        state_ = State::Running;
      }
    }

    virtual void Stop()
    {
      if (task_handle_ != nullptr && state_ != State::Stopped)
      {
        // 请求停止，等待任务自行退出
        state_ = State::Stopped;
        
        // 等待任务真正结束 (eDeleted 或 eInvalid)
        constexpr int kMaxWaitMs = 5000;
        constexpr int kPollIntervalMs = 10;
        int waited = 0;
        
        while (waited < kMaxWaitMs)
        {
          eTaskState task_state = eTaskGetState(task_handle_);
          if (task_state == eDeleted || task_state == eInvalid)
          {
            break;
          }
          vTaskDelay(pdMS_TO_TICKS(kPollIntervalMs));
          waited += kPollIntervalMs;
        }
        
        // 如果任务仍未结束，强制删除
        if (waited >= kMaxWaitMs)
        {
          vTaskDelete(task_handle_);
        }
        
        task_handle_ = nullptr;
      }
    }
    
    virtual void Delay(int ms)
    {
      vTaskDelay(pdMS_TO_TICKS(ms));
    }

    virtual void TaskFunc() = 0;
  };
//!--------------------------------------------------------------------------------------------------------------------
  template <typename DataType>
  class QueueAbility : public Ability
  {
  public:
    QueueHandle_t queue_handle_;
    
    QueueAbility(AbilityType type, int id, const std::string_view &name) : Ability(type, id, name)
    {
      queue_handle_ = xQueueCreate(10, sizeof(DataType));
    }

    virtual ~QueueAbility() 
    {
      if (queue_handle_ != nullptr)
      {
        vQueueDelete(queue_handle_);
        queue_handle_ = nullptr;
      }
    }

    void Send(const DataType &data, int ms)
    {
      if (queue_handle_ != nullptr)
      {
        xQueueSend(queue_handle_, &data, pdMS_TO_TICKS(ms));
      }
    }

    void SendFont(const DataType &data, int ms)
    {
      if (queue_handle_ != nullptr)
      {
        xQueueSendToFront(queue_handle_, &data, pdMS_TO_TICKS(ms));
      }
    }

    void SendBack(const DataType &data, int ms)
    {
      if (queue_handle_ != nullptr)
      {
        xQueueSendToBack(queue_handle_, &data, pdMS_TO_TICKS(ms));
      }
    }

    void OverWrite(const DataType &data)
    {
      if (queue_handle_ != nullptr)
      {
        xQueueOverwrite(queue_handle_, &data);
      }
    }

    void Receive(DataType &data, int ms)
    {
      if (queue_handle_ != nullptr)
      {
        xQueueReceive(queue_handle_, &data, pdMS_TO_TICKS(ms));
      }
    }

    void Clear()
    {
      if (queue_handle_ != nullptr)
      {
        xQueueReset(queue_handle_);
      }
    }

    void Peek(DataType &data, int ms)
    {
      if (queue_handle_ != nullptr)
      {
        xQueuePeek(queue_handle_, &data, pdMS_TO_TICKS(ms));
      }
    }

    void Delete()
    {
      if (queue_handle_ != nullptr)
      {
        vQueueDelete(queue_handle_);
        queue_handle_ = nullptr;
      }
    }
  };
//!--------------------------------------------------------------------------------------------------------------------
  class ServiceAbility : public TaskAbility
  {
  public:

    enum class State
    {
      HasRequest,
      Processing,
      Idle,
      Stopped,
    };

    State state_{State::Stopped};

    bool HasRequest() const { return state_ == State::HasRequest; }
    bool IsProcessing() const { return state_ == State::Processing; }
    bool IsIdle() const { return state_ == State::Idle; }
    bool IsStopped() const { return state_ == State::Stopped; }

    EventGroupHandle_t event_group_;

// To Self

    virtual void TaskFunc() override
    {
      for(;;)
      {
        EventBits_t bits = xEventGroupWaitBits(event_group_, 0x01, pdTRUE, pdFALSE, portMAX_DELAY);
        if (bits & 0x01)
        {
          state_ = State::Processing;
          Process();
          state_ = State::Idle;
        }
      }
    }

// To Other Ability or External Call

    ServiceAbility(AbilityType type, int id, const std::string_view &name) : TaskAbility(type, id, name)
    {
      state_ = State::Stopped;
      event_group_ = xEventGroupCreate();
    }

    virtual ~ServiceAbility()
    {
      if (event_group_ != nullptr)
      {
        vEventGroupDelete(event_group_);
        event_group_ = nullptr;
      }
    }

    virtual void Start() override
    {
      if(State::Stopped != state_)
      {
        return; 
      }
      TaskAbility::Start();
      state_ = State::Idle;
    }

    virtual void Stop() override
    {
      if(State::Stopped == state_)
      {
        return; 
      }
      TaskAbility::Stop();
      state_ = State::Stopped;
    }

    virtual void RequestReady()
    {
      if (state_ == State::Idle)
      {
        state_ = State::HasRequest;
        xEventGroupSetBits(event_group_, 0x01);
      }
    }
// To Override
    virtual void Process() = 0;
    virtual void Request() = 0;
  };
} // namespace monncake