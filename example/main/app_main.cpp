#include <string>
#include <vector>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>

#include "service.hpp"
#include "freertps.hpp"

#if CONFIG_WRAPPER_ESP32_BOARD_M5STACK_CORE_S3
#include "board/m5stack/core-s3.hpp"
using namespace wrapper;
static void board_init(void *arg)
{
  M5StackCoreS3& board = M5StackCoreS3::GetInstance();

  board.Init();
  board.GetLvglPort().Test();
  vTaskDelete(nullptr);
}
#endif

#if CONFIG_WRAPPER_ESP32_BOARD_M5STACK_TAB5
#include "board/m5stack/tab5.hpp"
using namespace wrapper;
static void board_init(void *arg)
{
  M5StackTab5& board = M5StackTab5::GetInstance();

  board.Init();
  board.SetDisplayBacklight(true);
  board.SetDisplayBrightness(80);

  board.GetLvglPort().Lock(0);
  lv_obj_t* scr = lv_scr_act();
  lv_obj_t* label = lv_label_create(scr);
  lv_label_set_text(label, "Hello, M5Stack Tab5!");
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
  board.GetLvglPort().Unlock();
  vTaskDelete(nullptr);
}
#endif

extern "C" void app_main()
{
  // nvs init
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);

  xTaskCreate(board_init, "board_init", 8192, nullptr, 5, nullptr);
}
