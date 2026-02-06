#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <atomic>
#include <string>
#include <string_view>
#include <type_traits>
#include <optional>

namespace app {

/**
 * @brief Configuration for the Service
 */
struct ServiceConfig {
    const char* name = "Service";
    uint32_t stack_size = 4096;
    UBaseType_t priority = 5;
    BaseType_t core_id = 1; // 0, 1, or tskNO_AFFINITY
    UBaseType_t request_queue_depth = 10;
    UBaseType_t response_queue_depth = 10;
};

/**
 * @brief Abstract Service Class (C++17)
 * 
 * A thread-safe, FreeRTOS-integrated service framework.
 * Implements a Request-Response pattern using a dedicated worker task.
 * 
 * @tparam ReqT Request type. Must be safe for xQueueSend (POD or pointer/smart_ptr).
 * @tparam ResT Response type. Must be safe for xQueueSend (POD or pointer/smart_ptr).
 */
template <typename ReqT, typename ResT>
class Service {
public:
    enum class State {
        Stopped,
        Idle,
        Processing
    };

    Service() = default;
    virtual ~Service() {
        Stop();
    }

    // Disable copy/move to prevent resource issues
    Service(const Service&) = delete;
    Service& operator=(const Service&) = delete;

    /**
     * @brief Start the service task
     * @param config Service configuration
     * @return true if started successfully
     */
    bool Start(const ServiceConfig& config) {
        if (state_ != State::Stopped) {
            return false;
        }

        config_ = config;
        should_stop_ = false;

        // Create Queues
        // Note: FreeRTOS queues use memcpy. Ensure T is trivially copyable or a pointer.
        request_queue_ = xQueueCreate(config_.request_queue_depth, sizeof(ReqT));
        response_queue_ = xQueueCreate(config_.response_queue_depth, sizeof(ResT));

        if (!request_queue_ || !response_queue_) {
            Cleanup();
            return false;
        }

        // Create Task
        BaseType_t ret = xTaskCreatePinnedToCore(
            TaskWrapper,
            config_.name,
            config_.stack_size,
            this,
            config_.priority,
            &task_handle_,
            config_.core_id
        );

        if (ret != pdPASS) {
            Cleanup();
            return false;
        }

        state_ = State::Idle;
        return true;
    }

    /**
     * @brief Stop the service
     */
    void Stop() {
        should_stop_ = true;
        
        // Wait for task to finish
        if (task_handle_) {
            // Check if we are stopping from within the task itself (should avoid deadlock)
            if (xTaskGetCurrentTaskHandle() != task_handle_) {
                 // Simple polling wait for task deletion
                 // In a real system, might use a notification or join mechanism
                 // Here we assume the task loop checks should_stop_ and exits.
                 // We can't vTaskDelete(task_handle_) immediately if it's running.
                 
                 // Wait for state to become Stopped
                 const int max_retries = 100;
                 for (int i = 0; i < max_retries && state_ != State::Stopped; ++i) {
                     vTaskDelay(pdMS_TO_TICKS(10));
                 }
            }
        }

        Cleanup();
        state_ = State::Stopped;
    }

    /**
     * @brief Send a request to the service
     * @param req The request object
     * @param wait_ms Max time to wait in ms (-1 for portMAX_DELAY)
     * @return true if request enqueued
     */
    bool Request(const ReqT& req, int wait_ms = -1) {
        if (state_ == State::Stopped) return false;
        if (!request_queue_) return false;

        TickType_t ticks = (wait_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(wait_ms);

        // Safety check for size mismatch if types changed (unlikely in template)
        if (xQueueSend(request_queue_, &req, ticks) == pdTRUE) {
            return true;
        }
        return false;
    }

    /**
     * @brief Wait for a response from the service
     * @param resp [out] The response object
     * @param wait_ms Max time to wait in ms (-1 for portMAX_DELAY)
     * @return true if response received
     */
    bool WaitResponse(ResT& resp, int wait_ms = -1) {
        if (!response_queue_) return false;

        TickType_t ticks = (wait_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(wait_ms);

        if (xQueueReceive(response_queue_, &resp, ticks) == pdTRUE) {
            return true;
        }
        return false;
    }

    State GetState() const {
        return state_.load(std::memory_order_relaxed);
    }

    bool IsRunning() const {
        return state_ != State::Stopped;
    }

protected:
    /**
     * @brief Pure virtual function to process a request.
     * Implement this in your concrete Service class.
     * @param req Input request
     * @return ResT Output response
     */
    virtual ResT Process(const ReqT& req) = 0;

    /**
     * @brief Optional: Called when task starts
     */
    virtual void OnStart() {}

    /**
     * @brief Optional: Called when task stops
     */
    virtual void OnStop() {}

private:
    ServiceConfig config_;
    TaskHandle_t task_handle_ = nullptr;
    QueueHandle_t request_queue_ = nullptr;
    QueueHandle_t response_queue_ = nullptr;
    
    std::atomic<State> state_{State::Stopped};
    std::atomic<bool> should_stop_{false};

    void Cleanup() {
        if (request_queue_) {
            vQueueDelete(request_queue_);
            request_queue_ = nullptr;
        }
        if (response_queue_) {
            vQueueDelete(response_queue_);
            response_queue_ = nullptr;
        }
        task_handle_ = nullptr;
    }

    static void TaskWrapper(void* param) {
        auto* self = static_cast<Service*>(param);
        self->Run();
        // Self-deletion
        vTaskDelete(nullptr);
    }

    void Run() {
        OnStart();
        
        ReqT req;
        while (!should_stop_) {
            // Wait for request
            // We use a timeout to check should_stop_ periodically
            if (xQueueReceive(request_queue_, &req, pdMS_TO_TICKS(100)) == pdTRUE) {
                state_ = State::Processing;
                
                // Process the request
                ResT res = this->Process(req);
                
                // Send response
                // Loop to allow checking should_stop_ if queue is full
                while (!should_stop_) {
                    if (xQueueSend(response_queue_, &res, pdMS_TO_TICKS(100)) == pdTRUE) {
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

} // namespace app
