/*
*************************************************************
* @file /project/node/src/main.c
* @author Geordie Pearson - 45798232
* @date 20-05-2022
* @brief main function for nodes
*************************************************************
*/

#include <zephyr.h>
#include <drivers/sensor.h>
#include <devicetree.h>
#include <device.h>

#include "node_sensors.h"
#include "node_ble.h"

#if MOBILE_NODE == 1
K_THREAD_DEFINE(handle_sensor_id, SENSORS_STACKSIZE, handle_sensor_mobile, NULL, NULL, NULL,
		SENSORS_PRIORITY, 0, 0);
// K_THREAD_DEFINE
#else
// K_THREAD_DEFINE
// K_THREAD_DEFINE
#endif
