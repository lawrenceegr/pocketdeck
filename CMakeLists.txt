zephyr_library()
zephyr_include_directories(${APPLICATION_SOURCE_DIR}/include)
zephyr_library_sources_ifdef(CONFIG_ZMK_BEHAVIOR_BUZZER src/behaviors/behavior_buzzer.c)
