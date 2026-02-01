idf_component_register(
  SRCS "app_main.cpp"
  INCLUDE_DIRS "."
  REQUIRES nvs_flash driver esp_driver_i2c esp_wifi esp_event
)
