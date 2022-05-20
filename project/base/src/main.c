/**
 * 
 * base node main.c file for project athena-green CSSE4011
 * 
 * Copyright Haoxi Tan & Geordie Pearson 2022
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <drivers/gpio.h>
#include <usb/usb_device.h>
#include <drivers/uart.h>
#include <shell/shell.h>
#include <device.h>
#include <drivers/pwm.h>

#include <zephyr/types.h>

// string things
#include <stddef.h>
// #include <toolchain.h>
#include <logging/log.h>

#include <stdio.h>
#include <stdint.h>

#include "base_ble.h"

// init logging

/* Define logging module */

LOG_MODULE_REGISTER(base_module, LOG_LEVEL_DBG);



/* Debug Thread Stack size */
#define THREAD_BLE_LED_STACK 512
#define THREAD_BLE_BASE_STACK 4094
#define THREAD_BLE_JSON_STACK 4096
/* Debug Thread Priority */
#define THREAD_PRIORITY_BLE_LED 10
#define THREAD_PRIORITY_BLE_BASE -2
#define THREAD_PRIORITY_JSON_SAMPLING 5



void main(void)
{


    int ret;

	/* Setup DTR */
    // const struct device *shell_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_shell_uart));
    // uint32_t dtr = 0;

    /* Enable the USB Driver */
    if (usb_enable(NULL))  { // if this doesn succeed no LED0 will be on
    	return;
    }
    led_init();

   
	// while (1) {
	// 	// ret = gpio_pin_toggle_dt(&led);
	// 	gpio_pin_set_dt(&led, 1);
	// 	// if (ret < 0) {
	// 		// return;
	// 	// }
	// 	k_msleep(SLEEP_TIME_MS);
	// }
}



K_THREAD_DEFINE(ble_base, THREAD_BLE_BASE_STACK, thread_ble_base, NULL, NULL, NULL, THREAD_PRIORITY_BLE_BASE, 0, 0);
// K_THREAD_DEFINE(ble_led, THREAD_BLE_LED_STACK, thread_ble_led, NULL, NULL, NULL, THREAD_PRIORITY_BLE_LED, 0, 0);
// K_THREAD_DEFINE(ble_json_sampling, THREAD_BLE_JSON_STACK, thread_ble_json_output, NULL, NULL, NULL, THREAD_PRIORITY_JSON_SAMPLING, 0, 0);
