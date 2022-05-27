// Authoor: Geordie Pearson
/*
*************************************************************
* @file oslib/node_driver/node_sensors/node_sensors.h
* @author Geordie Pearson - 45798232
* @date 19-05-2022
* @brief sensor functionality for nodes
*************************************************************
*/

#ifndef NODE_SENSORS_H
#define NODE_SENSORS_H

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <drivers/sensor.h>

#if MOBILE_NODE == 1
	/* Device Tree Macros for Mobile Node */
	#define LED_RED_NODE DT_NODELABEL(led0)
	#define LED_GREEN_NODE DT_NODELABEL(led1)
	#define LED_BLUE_NODE DT_NODELABEL(led2)
	#define BUTTON_NODE DT_NODELABEL(button0)
	#define EXPANDER_NODE DT_NODELABEL(sx1509b)
	#define LIS2DH_NODE DT_NODELABEL(lis2dh12)
	#define MPU_NODE DT_NODELABEL(mpu9250)
	#define VDD_MPU_NODE DT_NODELABEL(mpu_pwr)
#else
	/* Device Tree Macros for Static Node */
	// #define LED_RED_NODE DT_NODELABEL(led0)
#endif

#define VDD_PWR_CTRL_PIN 30
#define MPU_PWR_CTRL_PIN 8

/* Defines for led indexing */
#define LED_RED_INDEX 0
#define LED_GREEN_INDEX 1
#define LED_BLUE_INDEX 2

/* Defines thread specfics */
#define SENSORS_STACKSIZE 1024
#define SENSORS_PRIORITY 7
#define SENSORS_SLEEP 1000

/* Declares sempahore for data access */
extern struct k_sem sensor_sem;

/* Defines type to store all io data */
typedef struct io_data {
	uint8_t led_states[3];
	uint8_t button_state;
	int buzzer;
	int dc;
	int sleep_time;
	int send_time;
	int rec_time;
	int rec_num;
} io_data;

/* Defines type to store all sensor data */
typedef struct sensor_data {
	double temperature;
	double humidity;	
	double pressure;
	double voc;
	double x_accel;
	double y_accel;
	double z_accel;
	int dir;
} sensor_data;

/* Function Prototypes */
// Initializes the given LED.
// Parameters:
// 	- data: The data struct to store the led state
// 	- led_num: The number of the led to init
// Returns:
// 	- 0 if the initialization succeeds, otherwise -1
int init_led(io_data* data, int led_num);

// Intializes the button for the mobile node.
// Parameters:
// 	- data: The data struct to store the button state
// Returns:
// 	- 0 if the initialization succeeds, otherwise -1
int init_button(io_data* data);

// Toggles the state of the given LED.
// Parameters:
// 	- data: The data to store the led state in
// 	- led_num: The number of the led to toggle
void toggle_led(io_data* data, int led_num);

// Sets the given led to the given value.
// Parameters:
// 	- value: The value to set the led to
// 	- data: The data to store the led state in
// 	- led_num: The number of the led to set
void set_led(int value, io_data* data, int led_num);

// Button callback function
// Parameters:
//	- dev: The button device
//	- cb: The gpio callbackd data struct
//	- pins: The pin of the button
void toggle_button(const struct device* dev, struct gpio_callback* cb, uint32_t pins);

// Reads the given sensor and stores the data in the given data sensor_data struct.Reads given
// sensor based on the provided number of reads and the types of reads to conduct.
// Parameters:
// 	- dev: The sensor device to read
// 	- reading_types: The types of data to read from the sensor
// 	- num_sensors: The number of sensor reading to get from the given sensor
// 	- data: The data received from the sensor
// Returns:
//	0 if the read succeeds, -1 otherwise.
int read_sensor(const struct device* dev, const int* reading_types, int num_sensors,
		 sensor_data* data);

// Gets the current state of the button.
// Returns:
// 	The current state of the button.
uint8_t get_button_state(void);

// Converts the acceleration measured by the mobile node into a step count.
// Parameters:
// 	- data: The sensor data with acceleration information
// 	- prev_values: The previous acceleration values
// Returns:
// 	The number of steps that have occured
uint8_t acceleration_to_step(sensor_data data, int* prev_values);

// Converts the acceleration measured by the mobile node into a direction.
// Parameters:
// 	- data: The sensor data with acceleration information
// Returns:
// 	The current bearing of the mobile node
uint8_t acceleration_to_direction(sensor_data* data);

// Function that operates as thread opening point to handle all mobile sensor interactions.
void handle_sensor_mobile(void);

#endif
