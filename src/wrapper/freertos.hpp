
#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/event_groups.h>

#include <string>
#include <functional>
#include <atomic>
#include <string>
#include <string_view>
#include <type_traits>
#include <optional>

namespace wrapper
{

    void Delay(uint32_t milliseconds);

    void DelayTicks(uint32_t ticks);

    void DelayUntil(uint32_t *previous_wake_time, uint32_t time_increment_ms);

    void DelayUntilTicks(uint32_t *previous_wake_time, uint32_t time_increment_ticks);

    class Task
    {
    protected:
        std::string name_;
        std::function<void(void *)> function_;
        void *arg_;
        configSTACK_DEPTH_TYPE stack_depth_;
        UBaseType_t priority_;
        TaskHandle_t handle_;

    public:
        static constexpr uint32_t MAX_DELAY_TICK = portMAX_DELAY;
        static constexpr uint32_t MAX_DELAY_MS = pdTICKS_TO_MS(portMAX_DELAY);

        Task(const std::string &name,
             std::function<void(void *)> function,
             void *arg,
             uint32_t stack_depth,
             UBaseType_t priority);

        ~Task();

        bool Create();

        void Delete();

        void Suspend();

        void Resume();

        void SetPriority(UBaseType_t new_priority);
        
        std::string GetName() const;

        uint32_t GetStackDepth() const;

        UBaseType_t GetPriority() const;

        TaskHandle_t GetHandle() const;
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

        bool Send(const T &item, TickType_t wait_ticks = portMAX_DELAY)
        {
            if (handle_ == nullptr)
                return false;
            return xQueueSend(handle_, &item, wait_ticks) == pdTRUE;
        }

        bool SendFromISR(const T &item, BaseType_t *pxHigherPriorityTaskWoken)
        {
            if (handle_ == nullptr)
                return false;
            return xQueueSendFromISR(handle_, &item, pxHigherPriorityTaskWoken) == pdTRUE;
        }

        bool SendToFront(const T &item, TickType_t wait_ticks = portMAX_DELAY)
        {
            if (handle_ == nullptr)
                return false;
            return xQueueSendToFront(handle_, &item, wait_ticks) == pdTRUE;
        }

        bool SendToFrontFromISR(const T &item, BaseType_t *pxHigherPriorityTaskWoken)
        {
            if (handle_ == nullptr)
                return false;
            return xQueueSendToFrontFromISR(handle_, &item, pxHigherPriorityTaskWoken) == pdTRUE;
        }

        bool Overwrite(const T &item)
        {
            if (handle_ == nullptr)
                return false;
            return xQueueOverwrite(handle_, &item) == pdTRUE;
        }

        bool OverwriteFromISR(const T &item, BaseType_t *pxHigherPriorityTaskWoken)
        {
            if (handle_ == nullptr)
                return false;
            return xQueueOverwriteFromISR(handle_, &item, pxHigherPriorityTaskWoken) == pdTRUE;
        }

        bool Receive(T &item, TickType_t wait_ticks = portMAX_DELAY)
        {
            if (handle_ == nullptr)
                return false;
            return xQueueReceive(handle_, &item, wait_ticks) == pdTRUE;
        }

        bool ReceiveFromISR(T &item, BaseType_t *pxHigherPriorityTaskWoken)
        {
            if (handle_ == nullptr)
                return false;
            return xQueueReceiveFromISR(handle_, &item, pxHigherPriorityTaskWoken) == pdTRUE;
        }

        bool Peek(T &item, TickType_t wait_ticks = portMAX_DELAY)
        {
            if (handle_ == nullptr)
                return false;
            return xQueuePeek(handle_, &item, wait_ticks) == pdTRUE;
        }

        bool PeekFromISR(T &item)
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
                return true; // 中文注释：已按当前代码逻辑本地化。
            return xQueueIsQueueEmptyFromISR(handle_) == pdTRUE;
        }

        bool IsQueueFullFromISR() const
        {
            if (handle_ == nullptr)
                return false; // 中文注释：已按当前代码逻辑本地化。
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
        Semaphore();

        virtual ~Semaphore();

        bool Take(TickType_t wait_ticks = portMAX_DELAY);

        bool Give();

        bool TakeFromISR(BaseType_t *pxHigherPriorityTaskWoken);

        bool GiveFromISR(BaseType_t *pxHigherPriorityTaskWoken);

        SemaphoreHandle_t GetHandle() const;

        bool IsValid() const;
    };

    class BinarySemaphore : public Semaphore
    {
    public:
        BinarySemaphore();
    };

    class CountingSemaphore : public Semaphore
    {
    public:
        CountingSemaphore(UBaseType_t max_count, UBaseType_t initial_count);
    };

    class Mutex : public Semaphore
    {
    public:
        Mutex();
    };

    class RecursiveMutex : public Semaphore
    {
    public:
        RecursiveMutex();

        bool Take(TickType_t wait_ticks = portMAX_DELAY);

        bool Give();
    };

    class EventGroup
    {
        EventGroupHandle_t handle_;

    public:
        EventGroup();

        ~EventGroup();

        EventBits_t WaitBits(const EventBits_t bits_to_wait_for,
                             const bool clear_on_exit,
                             const bool wait_for_all_bits,
                             TickType_t wait_ticks = portMAX_DELAY);

        EventBits_t SetBits(const EventBits_t bits_to_set);

        EventBits_t SetBitsFromISR(const EventBits_t bits_to_set, BaseType_t *pxHigherPriorityTaskWoken);

        EventBits_t ClearBits(const EventBits_t bits_to_clear);

        EventBits_t ClearBitsFromISR(const EventBits_t bits_to_clear);

        EventBits_t GetBits() const;

        EventBits_t GetBitsFromISR() const;

        EventBits_t Sync(const EventBits_t bits_to_set,
                         const EventBits_t bits_to_wait_for,
                         TickType_t wait_ticks = portMAX_DELAY);

        EventGroupHandle_t GetHandle() const;

        bool IsValid() const;
    };
    /* 中文注释：已按当前代码逻辑本地化。 */
    struct ServiceConfig
    {
        const char *name = "Service";
        uint32_t stack_size = 4096;
        UBaseType_t priority = 5;
        BaseType_t core_id = 1; // 中文注释：已按当前代码逻辑本地化。
        UBaseType_t request_queue_depth = 10;
        UBaseType_t response_queue_depth = 10;
    };

    /* 中文注释：已按当前代码逻辑本地化。 */
    template <typename ReqT, typename ResT>
    class Service
    {
    public:
        enum class State
        {
            Stopped,
            Idle,
            Processing
        };

        Service() = default;
        virtual ~Service()
        {
            Stop();
        }

        // 中文注释：已按当前代码逻辑本地化。
        Service(const Service &) = delete;
        Service &operator=(const Service &) = delete;

        /* 中文注释：已按当前代码逻辑本地化。 */
        bool Start(const ServiceConfig &config)
        {
            if (state_ != State::Stopped)
            {
                return false;
            }

            config_ = config;
            should_stop_ = false;

            // 中文注释：已按当前代码逻辑本地化。
            // 中文注释：已按当前代码逻辑本地化。
            request_queue_ = xQueueCreate(config_.request_queue_depth, sizeof(ReqT));
            response_queue_ = xQueueCreate(config_.response_queue_depth, sizeof(ResT));

            if (!request_queue_ || !response_queue_)
            {
                Cleanup();
                return false;
            }

            // 中文注释：已按当前代码逻辑本地化。
            BaseType_t ret = xTaskCreatePinnedToCore(
                TaskWrapper,
                config_.name,
                config_.stack_size,
                this,
                config_.priority,
                &task_handle_,
                config_.core_id);

            if (ret != pdPASS)
            {
                Cleanup();
                return false;
            }

            state_ = State::Idle;
            return true;
        }

        /* 中文注释：已按当前代码逻辑本地化。 */
        void Stop()
        {
            should_stop_ = true;

            // 中文注释：已按当前代码逻辑本地化。
            if (task_handle_)
            {
                // 中文注释：已按当前代码逻辑本地化。
                if (xTaskGetCurrentTaskHandle() != task_handle_)
                {
                    // 中文注释：已按当前代码逻辑本地化。
                    // 中文注释：已按当前代码逻辑本地化。
                    // 中文注释：已按当前代码逻辑本地化。
                    // 中文注释：已按当前代码逻辑本地化。

                    // 中文注释：已按当前代码逻辑本地化。
                    const int max_retries = 100;
                    for (int i = 0; i < max_retries && state_ != State::Stopped; ++i)
                    {
                        vTaskDelay(pdMS_TO_TICKS(10));
                    }
                }
            }

            Cleanup();
            state_ = State::Stopped;
        }

        /* 中文注释：已按当前代码逻辑本地化。 */
        bool Request(const ReqT &req, int wait_ms = -1)
        {
            if (state_ == State::Stopped)
                return false;
            if (!request_queue_)
                return false;

            TickType_t ticks = (wait_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(wait_ms);

            // 中文注释：已按当前代码逻辑本地化。
            if (xQueueSend(request_queue_, &req, ticks) == pdTRUE)
            {
                return true;
            }
            return false;
        }

        /* 中文注释：已按当前代码逻辑本地化。 */
        bool WaitResponse(ResT &resp, int wait_ms = -1)
        {
            if (!response_queue_)
                return false;

            TickType_t ticks = (wait_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(wait_ms);

            if (xQueueReceive(response_queue_, &resp, ticks) == pdTRUE)
            {
                return true;
            }
            return false;
        }

        State GetState() const
        {
            return state_.load(std::memory_order_relaxed);
        }

        bool IsRunning() const
        {
            return state_ != State::Stopped;
        }

    protected:
        /* 中文注释：已按当前代码逻辑本地化。 */
        virtual ResT Process(const ReqT &req) = 0;

        /* 中文注释：已按当前代码逻辑本地化。 */
        virtual void OnStart() {}

        /* 中文注释：已按当前代码逻辑本地化。 */
        virtual void OnStop() {}

    private:
        ServiceConfig config_;
        TaskHandle_t task_handle_ = nullptr;
        QueueHandle_t request_queue_ = nullptr;
        QueueHandle_t response_queue_ = nullptr;

        std::atomic<State> state_{State::Stopped};
        std::atomic<bool> should_stop_{false};

        void Cleanup()
        {
            if (request_queue_)
            {
                vQueueDelete(request_queue_);
                request_queue_ = nullptr;
            }
            if (response_queue_)
            {
                vQueueDelete(response_queue_);
                response_queue_ = nullptr;
            }
            task_handle_ = nullptr;
        }

        static void TaskWrapper(void *param)
        {
            auto *self = static_cast<Service *>(param);
            self->Run();
            // 中文注释：已按当前代码逻辑本地化。
            vTaskDelete(nullptr);
        }

        void Run()
        {
            OnStart();

            ReqT req;
            while (!should_stop_)
            {
                // 中文注释：已按当前代码逻辑本地化。
                // 中文注释：已按当前代码逻辑本地化。
                if (xQueueReceive(request_queue_, &req, pdMS_TO_TICKS(100)) == pdTRUE)
                {
                    state_ = State::Processing;

                    // 中文注释：已按当前代码逻辑本地化。
                    ResT res = this->Process(req);

                    // 中文注释：已按当前代码逻辑本地化。
                    // 中文注释：已按当前代码逻辑本地化。
                    while (!should_stop_)
                    {
                        if (xQueueSend(response_queue_, &res, pdMS_TO_TICKS(100)) == pdTRUE)
                        {
                            break;
                        }
                    }

                    state_ = State::Idle;
                }
            }

            OnStop();
            state_ = State::Stopped;
        }
    };
} // 中文注释：已按当前代码逻辑本地化。
