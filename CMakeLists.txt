# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion RISCV_RPI_2_0_0_5)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# Wygenerowany plik Cmake dla projektu Pico

cmake_minimum_required(VERSION 3.13)

# Dołącz ustawienia dla rozszerzenia VS Code
# Commented out to avoid configuration issues
# if(WIN32)
#     set(USERHOME $ENV{USERPROFILE})
# else()
#     set(USERHOME $ENV{HOME})
# endif()
# set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
# if (EXISTS ${picoVscode})
#     include(${picoVscode})
# endif()

# Ustawia płytkę na Pico W, zgodnie z informacją od użytkownika
set(PICO_BOARD pico2_w CACHE STRING "Board type")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dołącz SDK Raspberry Pi Pico
set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/pico-sdk")
include(pico_sdk_import.cmake)

project(led_test_sk9822 C CXX ASM)

# Inicjalizuj SDK
pico_sdk_init()

# Dodaj plik wykonywalny, wskazując na nowy plik PIO
add_executable(led_test_sk9822
    led_test_c.c
    sk9822.pio
)

# Konfiguracja USB/UART stdio
pico_enable_stdio_uart(led_test_sk9822 0)
pico_enable_stdio_usb(led_test_sk9822 1)

# Generuj nagłówek dla nowego programu PIO
pico_generate_pio_header(led_test_sk9822 ${CMAKE_CURRENT_LIST_DIR}/sk9822.pio)

# Dołącz potrzebne biblioteki
target_link_libraries(led_test_sk9822
    pico_stdlib
    hardware_pio
    hardware_clocks
    hardware_dma
    m
)

# Dodaj dodatkowe pliki wyjściowe (uf2, etc.)
pico_add_extra_outputs(led_test_sk9822)