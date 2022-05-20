# SPDX-License-Identifier: Apache-2.0

# SPDX-License-Identifier: Apache-2.0
#Setting to exclusively build for the dongle
set(BOARD nrf52840dongle_nrf52840)
set(CONF_FILE prj.conf bt.conf usb.conf shell.conf)

set(DTC_OVERLAY_FILE dtc_shell.overlay)

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ahu)

target_sources(app PRIVATE 
			src/main.c 
			../../oslib/base_drivers/base_ble/base_ble.c 
		)

#Add include_directories for libraries, path starts from this files location.
include_directories(
			inc/
                        ../../oslib/base_drivers/base_ble/
                       )



zephyr_library_include_directories(${ZEPHYR_BASE}/samples/bluetooth)
