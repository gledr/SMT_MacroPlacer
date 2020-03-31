project(placer_config)

add_custom_target(placer_config COMMAND ${placer_config_SOURCE_DIR}/install_placer.sh WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
