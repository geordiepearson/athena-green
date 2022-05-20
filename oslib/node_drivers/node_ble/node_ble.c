// Geordie Pearson
/*
*************************************************************
* @file oslib/node_driver/node_ble/node_ble.c
* @author Geordie Pearson - 45798232
* @date 20-05-2022
* @brief ble functionality for nodes
*************************************************************
*/

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <sys/byteorder.h>

#include "node_ble.h"

