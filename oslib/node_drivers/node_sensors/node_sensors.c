// Author: Geordie Pearson
/*
*************************************************************
* @file oslib/node_driver/node_sensors/node_sensors.c
* @author Geordie Pearson - 45798232
* @date 19-05-2022
* @brief sensor functionality for nodes
*************************************************************
*/


#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <drivers/sensor.h>
#include "node_sensors.h"

#if MOBILE_NODE == 1
	/* Device handles for IO peripherals */
	static const struct gpio_dt_spec thingy52_red_led = GPIO_DT_SPEC_GET(LED_RED_NODE, gpios);
	static const struct gpio_dt_spec thingy52_green_led = GPIO_DT_SPEC_GET(LED_GREEN_NODE, gpios);
	static const struct gpio_dt_spec thingy52_blue_led = GPIO_DT_SPEC_GET(LED_BLUE_NODE, gpios);
	static const struct gpio_dt_spec* leds[3] = {&thingy52_red_led, &thingy52_green_led,
						     &thingy52_blue_led};
	static const struct gpio_dt_spec thingy52_button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);
	static struct gpio_callback button_cb_data;

	/* Device handles for sensors */

#else
	/* Device handles for IO peripherals */
	static const struct gpio_dt_spec thingy52_red_led = GPIO_DT_SPEC_GET(LED_RED_NODE, gpios);
	static const struct gpio_dt_spec thingy52_green_led;
	static const struct gpio_dt_spec thingy52_blue_led;
	static const struct gpio_dt_spec* leds[3] = {&thingy52_red_led, &thingy52_green_led,
						     &thingy52_blue_led};
	static const struct gpio_dt_spec thingy52_button;
	static struct gpio_callback button_cb_data;

	/* Device handles for sensors */
	
#endif

/* Defines initial io and sensor states and intialises data managment objects */
io_data io = {{0, 0, 0}, 0, 0, 100, 10};
sensor_data data = {0, 0, 0, 0, 0, 0, 0};
K_SEM_DEFINE(sensor_sem, 1, 1);

/* Defines config struct for imu */
struct pwr_ctrl_cfg {
	const char* port;
	uint32_t pin;
}

static const struct pwr_ctlrl_cfg imu_pwr_ctrl_cfg = {
	//.port = ,
	//.pin
};

int init_led(io_data* data, int led_num) {
	int ret = -1;
	if (!device_is_ready(leds[led_num]->port)) {
		return ret;
	}

	ret = gpio_pin_configure_dt(leds[led_num], GPIO_OUTPUT_ACTIVE);
	set_led(0, data, led_num);
	return ret;
}

int init_button(io_data* data) {
	int ret = -1;
	if (!device_is_ready(thingy52_button.port)) {
		return ret;
	}

	ret = gpio_pin_configure_dt(&thingy52_button, GPIO_INPUT);

	// Initialises interrupt and callback
	ret = gpio_pin_interrupt_configure_dt(&thingy52_button, GPIO_INT_EDGE_TO_ACTIVE);
	gpio_init_callback(&button_cb_data, toggle_button, BIT(thingy52_button.pin));
	gpio_add_callback(thingy52_button.port, &button_cb_data);
	data->button_state = 0;
	return ret;
}

int init_imu(const struct device* dev) {
	const struct pwr_ctrl_cfg* cfg = dev->config;
	const struct device* gpio;

	gpio = device_get_binding(cfg->port);
	if (!gpio) {
		printk("Could not bind IMU device\n");
		return -1;
	}

	gpio_pin_configure(gpiom cfg->pin, GPIO_OUTPUT_HIGH);
	k_sleep(K_MSEC(1));
	return 0;
}

void toggle_led(io_data* data, int led_num) {
	gpio_pin_toggle_dt(leds[led_num]);
	data->led_states[led_num] ^= 1UL << 0;
	printk("Led %d toggled.\n", led_num);
}

void set_led(int value, io_data* data, int led_num) {
	gpio_pin_set_dt(leds[led_num], value);
	data->led_states[led_num] = value;
	printk("Led %d set to %d.\n", led_num, value);
}

void toggle_button(const struct device* dev, struct gpio_callback* cb, uint32_t pins) {
	printk("Button pressed.\n");
}

int read_sensor(const struct device* dev, const int* reading_types, int num_sensors,
		sensor_data* data) {
	int ret = -1;
	struct sensor_value sensor_reading;

	if (sensor_sample_fetch(dev) < 0) {
		printk("Device %s cannot be read.\n", dev->name);
		return ret;
	}

	for(int i = 0; i < num_sensors; i++) {
		// Handles accelereation separately due to different read size
		if (reading_types[i] == SENSOR_CHAN_ACCEL_XYZ) {
			struct sensor_value accel_reading[3];
			if (sensor_channel_get(dev, reading_types[i], accel_reading) < 0) {
				printk("Device acceleration failed read.\n");
				return ret;
			}
			data->x_accel = sensor_value_to_double(&accel_reading[0]);
			data->y_accel = sensor_value_to_double(&accel_reading[1]);
			data->z_accel = sensor_value_to_double(&accel_reading[2]);
		// Handles all other sensor read types for SCU
		} else {
			if (sensor_channel_get(dev, reading_types[i], &sensor_reading) < 0) {
			printk("Device %s failed read.\n", dev->name);
			return ret;
			}
			
			// Assigns data variables based on reading type
			if (reading_types[i] == SENSOR_CHAN_PRESS) {
				data->pressure = sensor_value_to_double(&sensor_reading);
			} else if (reading_types[i] == SENSOR_CHAN_AMBIENT_TEMP) {
				data->temperature = sensor_value_to_double(&sensor_reading);
			} else if (reading_types[i] == SENSOR_CHAN_HUMIDITY) {
				data->humidity = sensor_value_to_double(&sensor_reading);
			} else if (reading_types[i] == SENSOR_CHAN_VOC) {
				data->voc = sensor_value_to_double(&sensor_reading);
			} else {
				return ret;
			}
		}
	}
	return 0;
}

uint8_t get_button_state() {
	return gpio_pin_get_dt(&thingy52_button);
}

void handle_sensor_mobile() {
	for (int i = 0; i < 3; i++) {
		init_led(&io, i);
	} 
	init_button(&io);

	while(1) {
		k_sem_take(&sensor_sem, K_FOREVER);
	
		// read mpu
		// update buffer

		k_sem_give(&sensor_sem);
		k_msleep(SENSORS_SLEEP);
	}
}
