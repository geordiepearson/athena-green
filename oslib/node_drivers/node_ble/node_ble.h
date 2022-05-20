// Geordie Pearson
/*
*************************************************************
* @file oslib/node_drivers/node_ble/node_ble.h
* @author Geordie Pearson - 45798232
* @date 20-05-2022
* @brief ble functionality for nodes
*************************************************************
*/

#ifndef NODE_BLE_H
#define NODE_BLE_H

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <sys/byteorder.h>

/* Defines constants for bluetooth sleep times (ms) */

// Initialises bluetooth advertising
void init_bt(void);

// Operates as thread opening point to handle mobile node bt
void handle_bt_mobile(void);

// Operates as thread opening point to handle static node bt
void handle_bt_static(void);

#endif
