# This file is used for configuring the project for the CLion IDE

cmake_minimum_required(VERSION 4.0)
project(pebble_rain_map_finland C)

set(CMAKE_C_STANDARD 17)

set(TARGET_PEBBLE_PLATFORM emery)  # Pebble Time 2
#set(TARGET_PEBBLE_PLATFORM flint)  # Pebble 2 Duo

if(TARGET_PEBBLE_PLATFORM STREQUAL "emery")
   add_compile_definitions(PBL_COLOR)
   add_compile_definitions(PBL_TOUCH)
endif()

include_directories(src/c/modules)
include_directories(src/c/modules/comm)
include_directories(src/c/windows)

include_directories(build/${TARGET_PEBBLE_PLATFORM}/src)
include_directories(build/include)

# For these, create a symbolic link
# include/pebble -> /<sdk-install-path>/Pebble SDK/SDKs/current/sdk-core/pebble
include_directories(include/pebble/${TARGET_PEBBLE_PLATFORM}/include)

add_executable(pebble_rain_map_finland
  build/${TARGET_PEBBLE_PLATFORM}/src/resource_ids.auto.c
  build/${TARGET_PEBBLE_PLATFORM}/src/resource_ids.auto.h
  build/${TARGET_PEBBLE_PLATFORM}/appinfo.auto.c
  build/include/message_keys.auto.h
  build/src/message_keys.auto.c
  src/c/modules/comm/comm.c
  src/c/modules/comm/comm.h
  src/c/modules/comm/comm_coordinates.c
  src/c/modules/comm/comm_coordinates.h
  src/c/modules/comm/comm_radar_data.c
  src/c/modules/comm/comm_radar_data.h
  src/c/modules/logging.c
  src/c/modules/logging.h
  src/c/modules/radar_data.c
  src/c/modules/radar_data.h
  src/c/modules/radar_data_cache.c
  src/c/modules/radar_data_cache.h
  src/c/modules/state.c
  src/c/modules/state.h
  src/c/modules/types.h
  src/c/modules/utils.c
  src/c/modules/utils.h
  src/c/windows/main/draw.c
  src/c/windows/main/draw.h
  src/c/windows/main/draw_map.c
  src/c/windows/main/draw_map.h
  src/c/windows/main/main_window.c
  src/c/windows/main/main_window.h
  src/c/main.c
)