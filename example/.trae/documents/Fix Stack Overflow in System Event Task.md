Based on the crash log, the error is `***ERROR*** A stack overflow in task sys_evt has been detected`. This typically happens when an event handler callback consumes too much stack space, as these callbacks run in the context of the system event task (default loop).

In `WifiProvider::EventHandler`, we are using `self->m_logger.Info(...)`. If `Logger::Info` or its underlying `ESP_LOGI` consumes significant stack (e.g., formatting large strings, or if the logging implementation itself is heavy), it can overflow the default system event task stack, which is often relatively small (default around 2-4KB).

The plan is to:

1. **Increase the stack size of the default event loop**:

   * In `src/wrapper/wifi.cpp`, modify the `WifiProvider` constructor. Instead of `esp_event_loop_create_default()`, we should try to create it with explicit parameters if it's not already created, or we can just try to optimize the logging.

   * However, `esp_event_loop_create_default()` uses a fixed stack size defined in menuconfig (`CONFIG_ESP_SYSTEM_EVENT_TASK_STACK_SIZE`). We cannot change it programmatically if it's already created or if we use the default creation API without menuconfig changes.

   * **Alternative**: The `EventHandler` logic is running inside the system event loop task. We should minimize stack usage there.

2. **Optimize** **`WifiProvider::EventHandler`**:

   * The `EventHandler` allocates `wifi_config_t` on the stack (approx 700+ bytes) inside the `SC_EVENT_GOT_SSID_PSWD` case. This is quite large.

   * The logging calls also use stack for formatting.

   * **Fix**: We should verify if we can reduce stack usage.

   * However, the crash happens during `WIFI_EVENT_STA_DISCONNECTED` -> `m_logger.Warning(...)`. The stack overflow might be cumulative.

3. **Better Fix**: Since we cannot easily change the system event loop stack size without recompiling with different menuconfig, and the user might not want to touch sdkconfig yet, we should **avoid heavy operations in the event handler**.

   * The `WifiProvider::EventHandler` is currently doing synchronous logging and even logic like `esp_wifi_connect()` (which is fine) and `smartconfig` logic.

   * The most likely culprit is `wifi_config_t` (large struct) or the logging overhead.

   **Proposal**:

   1. Modify `src/wrapper/wifi.cpp`:

      * In `EventHandler`, ensure `wifi_config_t` is not allocated on the stack unless necessary. For `SC_EVENT_GOT_SSID_PSWD`, it is necessary, but maybe we can make it static or heap-allocated (though heap allocation in ISR-like context is discouraged, system event task is a task, so it's okay but slow).

      * More importantly, check if we can reduce the stack depth of `Logger::Info`.

   **Actually, looking at the logs**:

   ```
   W (2522) | MAIN |: WiFi Disconnected, retrying...
   ***ERROR*** A stack overflow in task sys_evt has been detected.
   ```

   The overflow happens right after or during the logging in `WIFI_EVENT_STA_DISCONNECTED`.

   **Immediate Action**:

   * The user's `app_main` creates a separate task `board_init` with 8192 stack. This is good for `test_station`.

   * But the *event handler* runs in `sys_evt` task (the system event loop task), NOT in `board_init`.

   * The default system event task stack size is usually 2304 bytes (ESP-IDF default). `wifi_config_t` is \~800 bytes. `Logger` overhead + `printf` overhead might push it over.

   **Plan**:

   1. Modify `src/wrapper/wifi.cpp`:

      * In `WifiProvider::EventHandler`:

        * For `SC_EVENT_GOT_SSID_PSWD`, use `std::unique_ptr` or `malloc/free` for `wifi_config_t` to move it off the stack.

        * Verify other parts of `EventHandler` don't have large stack variables.
   2. If that's not enough, we might need to guide the user to increase `CONFIG_ESP_SYSTEM_EVENT_TASK_STACK_SIZE` in `sdkconfig`, but code changes are preferred first.

   **Let's try moving** **`wifi_config_t`** **to heap in** **`EventHandler`** **first.**

   Wait, the crash trace shows `panic_abort` -> `vApplicationStackOverflowHook`.
   The log shows `WiFi Disconnected, retrying...` printed successfully. Then crash.
   This implies the crash might happen *after* logging, or during the next calls.
   Inside `WIFI_EVENT_STA_DISCONNECTED`:

   ```cpp
   self->m_logger.Warning("WiFi Disconnected, retrying...");
   esp_wifi_connect();
   xEventGroupClearBits(self->m_event_group, WIFI_CONNECTED_BIT);
   ```

   These look safe.

   However, look at `SC_EVENT_GOT_SSID_PSWD` block:

   ```cpp
   wifi_config_t wifi_config = {}; // This is allocated on stack!
   ```

   Even if this block is not executed (we are in `WIFI_EVENT_STA_DISCONNECTED`), the compiler might reserve stack space for `wifi_config` for the whole function scope depending on optimization levels.
   `wifi_config_t` is large.

   **Refined Plan**:

   1. Refactor `WifiProvider::EventHandler` in `src/wrapper/wifi.cpp`.
   2. Split the large handler into smaller helper functions or use scoping `{}` to help compiler reuse stack, OR better yet, use heap allocation for `wifi_config_t`.
   3. Actually, just scoping it properly or using `std::unique_ptr` / `new` is safer for stack usage in the system event loop.

