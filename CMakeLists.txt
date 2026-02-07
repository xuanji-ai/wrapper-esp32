set(INCLUDE_DIRS "src")

set(REQUIRES_ESP_IDF "log" "nvs_flash" "driver" "esp_lcd" "esp_wifi" "esp_event" "wifi_provisioning")
set(REQUIRES_ESP_REGISTRY "")
set(REQUIRES_PROJECT "")

set(REQUIRES ${REQUIRES_ESP_IDF} ${REQUIRES_ESP_REGISTRY} ${REQUIRES_PROJECT})
set(PRIV_REQUIRES "")

set(EMBED_FILES "")

# Common sources
file(GLOB_RECURSE SOURCES 
    "src/wrapper/*.cpp" 
    "src/device/*.cpp" 
    "src/app/*.cpp"
)

# Exclude display-dsi.cpp and ili9881c.cpp from common sources
list(FILTER SOURCES EXCLUDE REGEX ".*display-dsi\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*ili9881c\\.cpp$")

# Add PowerHub source
list(APPEND SOURCES "src/board/m5stack/powerhub.cpp")

# Board specific sources
if(IDF_TARGET STREQUAL "esp32s3")
    list(APPEND SOURCES "src/board/m5stack/core-s3.cpp")
elseif(IDF_TARGET STREQUAL "esp32p4")
    list(APPEND SOURCES "src/board/m5stack/tab5.cpp")
    # Add DSI display support for ESP32P4
    list(APPEND SOURCES "src/wrapper/display-dsi.cpp")
    list(APPEND SOURCES "src/device/ili9881c.cpp")
endif()

idf_component_register(
  SRCS ${SOURCES}
  INCLUDE_DIRS ${INCLUDE_DIRS}
  REQUIRES ${REQUIRES}
  PRIV_REQUIRES ${PRIV_REQUIRES}
  EMBED_FILES ${EMBED_FILES}
)
