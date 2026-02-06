#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <atomic>
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

    std::atomic<State> state_{State::Stopped};
    std::atomic<bool> should_stop_{false};

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

    // RAII: 禁止拷贝，确保 Handle 唯一性
    TaskAbility(const TaskAbility&) = delete;
    TaskAbility& operator=(const TaskAbility&) = delete;

    virtual ~TaskAbility() 
    {
        Stop();
    }

    virtual void Start()
    {
      if (task_handle_ == nullptr)
      {
        should_stop_ = false;
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
      should_stop_ = true;

      if (task_handle_ != nullptr && state_ != State::Stopped)
      {
        // 请求停止，等待任务自行退出
        
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
        state_ = State::Stopped;
      }
    }
    
    virtual void Delay(int ms)
    {
      vTaskDelay(pdMS_TO_TICKS(ms));
    }

    virtual void TaskFunc() = 0;
  };
//!--------------------------------------------------------------------------------------------------------------------
  class ServiceAbility : public Ability
  {
  public:

    enum class State
    {
      HasRequest,
      Processing,
      Idle,
      Stopped,
    };

    std::atomic<State> state_{State::Stopped};
    std::atomic<bool> should_stop_{false};
    
    // Task resources
    configSTACK_DEPTH_TYPE stack_size_{2048};
    UBaseType_t priority_{0};
    TaskHandle_t task_handle_{nullptr};
    EventGroupHandle_t event_group_{nullptr};

    bool HasRequest() const { return state_ == State::HasRequest; }
    bool IsProcessing() const { return state_ == State::Processing; }
    bool IsIdle() const { return state_ == State::Idle; }
    bool IsStopped() const { return state_ == State::Stopped; }

    void SetStackSize(int stack_size) { stack_size_ = static_cast<configSTACK_DEPTH_TYPE>(stack_size); }
    void SetPriority(int priority) { priority_ = static_cast<UBaseType_t>(priority); }
    TaskHandle_t GetTaskHandle() const { return task_handle_; }

    ServiceAbility(AbilityType type, int id, const std::string_view &name) : Ability(type, id, name)
    {
      state_ = State::Stopped;
      event_group_ = xEventGroupCreate();
    }

    // RAII
    ServiceAbility(const ServiceAbility&) = delete;
    ServiceAbility& operator=(const ServiceAbility&) = delete;

    virtual ~ServiceAbility()
    {
      Stop();
      if (event_group_ != nullptr)
      {
        vEventGroupDelete(event_group_);
        event_group_ = nullptr;
      }
    }

    virtual void Start()
    {
      if (task_handle_ == nullptr && state_ == State::Stopped)
      {
        should_stop_ = false;
        UBaseType_t result = xTaskCreate(
            [](void *param)
            {
              ServiceAbility *ability = static_cast<ServiceAbility *>(param);
              ability->ServiceLoop();
              vTaskDelete(nullptr);
            },
            name_.data(), stack_size_, this, priority_, &task_handle_);

        if (result == pdPASS)
        {
          state_ = State::Idle;
        }
      }
    }

    virtual void Stop()
    {
      should_stop_ = true;
      if (event_group_) {
          xEventGroupSetBits(event_group_, 0x01); // Wake up if waiting
      }
      
      if (task_handle_ != nullptr)
      {
         // Wait for task to exit
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

        if (waited >= kMaxWaitMs)
        {
          vTaskDelete(task_handle_);
        }
        
        task_handle_ = nullptr;
      }
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

    virtual void Process() = 0;

  private:
    void ServiceLoop()
    {
      while (!should_stop_)
      {
        EventBits_t bits = xEventGroupWaitBits(event_group_, 0x01, pdTRUE, pdFALSE, pdMS_TO_TICKS(1000));
        
        if (should_stop_) break;

        if (bits & 0x01)
        {
          state_ = State::Processing;
          Process();
          state_ = State::Idle;
        }
      }
      state_ = State::Stopped;
    }
  };

  class OpusDecodeService : public ServiceAbility
  {
  public:

    std::vector<uint8_t> input_buffer_;
    std::vector<int16_t> output_buffer_;

    OpusDecodeService(int id, const std::string_view &name) : ServiceAbility(AbilityType::Service, id, name)
    {
    }
    virtual ~OpusDecodeService() = default;
    virtual void Process() override
    {
      // Opus解码处理逻辑
      // 模拟处理时间
      vTaskDelay(pdMS_TO_TICKS(100));
    }

    void Request(const std::vector<uint8_t>& encoded_data)
    {
      // 存储或处理传入的encoded_data
      // 然后调用RequestReady以通知服务处理请求
      RequestReady();
    }

    void GetDecodedData(std::vector<int16_t>& out_data)
    {
      out_data = output_buffer_;
    }
  };

} // namespace monncake