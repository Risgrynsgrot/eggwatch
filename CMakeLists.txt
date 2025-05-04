# cmake_minimum_required(VERSION 3.13)

# include(pico_sdk_import.cmake)

# project(test_project C CXX ASM)
# set(CMAKE_C_STANDARD 11)
# set(CMAKE_CXX_STANDARD 17)
# pico_sdk_init()

# add_executable(test
# 	test.c
# )

# pico_enable_stdio_usb(test 1)
# pico_enable_stdio_uart(test 1)

# pico_add_extra_outputs(test)

# target_link_libraries(test pico_stdlib)

  
cmake_minimum_required(VERSION 3.13)

set(PICO_PLATFORM rp2350)
set(PICO_BOARD pico2_w)
include(pico_sdk_import.cmake)

set(target_proj demo_oled_spi_pico)
project(${target_proj} C CXX ASM)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections -Wl,--gc-sections")
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(${target_proj}
    main.c
)

target_include_directories(${target_proj} PRIVATE u8g2/csrc)
file(GLOB U8G2_SRC u8g2/csrc/*.c)
add_library(u8g2 ${U8G2_SRC})
target_link_libraries(${target_proj} u8g2)

if (PICO_CYW43_SUPPORTED)
    target_link_libraries(${target_proj} pico_cyw43_arch_none)
  endif()

target_include_directories(${target_proj} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_options(${target_proj} PRIVATE -Xlinker --print-memory-usage)
target_compile_options(${target_proj} PRIVATE -Wall -Wextra -DCFG_TUSB_DEBUG=1)
target_link_libraries(${target_proj}  pico_stdlib hardware_spi)
pico_add_extra_outputs(${target_proj})

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 1)
