# SPDX-FileCopyrightText: Copyright 2020-2026 OpenRTX Contributors
#
# SPDX-License-Identifier: GPL-3.0-or-later

#
# CMake configuration file for the Radtel RT-950 Pro OpenRTX target
#
cmake_minimum_required(VERSION 3.20.0)

target_sources(app
  PRIVATE
    ${OPENRTX_ROOT}/platform/mcu/AT32F403A/drivers/delays.c
    ${OPENRTX_ROOT}/platform/drivers/GPIO/gpio_at32f403a.c

    ${OPENRTX_ROOT}/platform/drivers/display/display_rt950pro.c
    ${OPENRTX_ROOT}/platform/drivers/keyboard/keyboard_rt950pro.c

    ${OPENRTX_ROOT}/platform/drivers/stubs/audio_stub.c
    ${OPENRTX_ROOT}/platform/drivers/stubs/radio_stub.c
    ${OPENRTX_ROOT}/platform/drivers/stubs/nvmem_stub.c
    ${OPENRTX_ROOT}/platform/drivers/stubs/cps_io_stub.c

    ${OPENRTX_ROOT}/platform/targets/rt950pro/board_init.c
    ${OPENRTX_ROOT}/platform/targets/rt950pro/hwconfig.c
    ${OPENRTX_ROOT}/platform/targets/rt950pro/platform.c
)

# Vendor bootloader model signatures, right after the vector table
zephyr_linker_sources(ROM_START SORT_KEY 0x3rt950 rt950pro_model_sig.ld)

target_include_directories(app
  PRIVATE
    ${OPENRTX_ROOT}/platform/targets/rt950pro
)
