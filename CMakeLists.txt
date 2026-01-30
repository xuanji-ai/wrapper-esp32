set(SRC_DIRS ".")
set(INCLUDE_DIRS ".")
set(REQUIRES_ESP_IDF "log" "driver" "esp_lcd")
set(REQUIRES_ESP_REGISTRY "")
set(REQUIRES_PROJECT "")
set(REQUIRES ${REQUIRES_ESP_IDF} ${REQUIRES_ESP_REGISTRY} ${REQUIRES_PROJECT})
set(PRIV_REQUIRES "")
set(EMBED_FILES "")

if(CONFIG_NIX_WRAPPER_BOARD_M5STACK_CORES3_C)
    list(APPEND SRC_DIRS "boards/m5stack_core_s3_c")
    list(APPEND INCLUDE_DIRS "boards/m5stack_core_s3_c")
elseif(CONFIG_NIX_WRAPPER_BOARD_M5STACK_CORES3_CPP)
    list(APPEND SRC_DIRS "boards/m5stack_core_s3_cpp")
    list(APPEND INCLUDE_DIRS "boards/m5stack_core_s3_cpp")
elseif(CONFIG_NIX_WRAPPER_BOARD_M5STACK_TAB5_C)
    list(APPEND SRC_DIRS "boards/m5stack_tab5_c")
    list(APPEND INCLUDE_DIRS "boards/m5stack_tab5_c")
elseif(CONFIG_NIX_WRAPPER_BOARD_M5STACK_TAB5_CPP)
    list(APPEND SRC_DIRS "boards/m5stack_tab5_cpp")
    list(APPEND INCLUDE_DIRS "boards/m5stack_tab5_cpp")
endif()

list(APPEND SRC_DIRS "devices")

idf_component_register(
  SRC_DIRS ${SRC_DIRS}
  INCLUDE_DIRS ${INCLUDE_DIRS}
  REQUIRES ${REQUIRES}
  PRIV_REQUIRES ${PRIV_REQUIRES}
  EMBED_FILES ${EMBED_FILES}
)
