#include <string>
#include <vector>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "wrapper/logger.hpp"
#include "wrapper/soc.hpp"
#include "wrapper/wifi.hpp"
#include "board/m5stack_core_s3_cpp/m5stack_core_s3.hpp"

#include "wifi_manager.h"

using namespace wrapper;

Logger logger_main("app_main");

WifiManagerConfig wifi_config = {
  .ssid_prefix = "M5StackS3",
  .language = "en-US",
  .station_scan_min_interval_seconds = 10,
  .station_scan_max_interval_seconds = 300,
};

static void board_init(void* arg)
{
  auto& wifi_mgr = WifiManager::GetInstance();
  wifi_mgr.Initialize(wifi_config);
  wifi_mgr.StartConfigAp();

  while (true) 
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

extern "C" void app_main()
{
  xTaskCreate(board_init, "board_init", 8192, nullptr, 5, nullptr);
};
