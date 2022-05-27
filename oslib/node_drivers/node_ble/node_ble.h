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
#define MOBILE_ADV_TYPE 0x42
#define STATIC_ADV_TYPE 0x43

/**
 * packet structure to advertise to nearly mobile and static nodes
 **/
struct mobile_ad {
	char m_id;
	char b1_id; 
	int8_t b1_rssi;
	char b2_id; 
	int8_t b2_rssi;
	char b3_id; 
	int8_t b3_rssi;
	int8_t speed;
	int8_t direction;
};

/**
 * packet structure to relay information between static nodes
 **/
struct static_ad {
	int8_t ttl; // initially a small value and packet should no longer be forwarded when this hits 0
	int8_t static_id; // static node id
	struct mobile_ad m_ad;
};

// define beacons tracked
#define BEACONS 3

//struct mobile_ad m_ad = {} 


// Initialises bluetooth advertising
void init_bt(void);

// Operates as thread opening point to handle mobile node bt
void handle_bt_mobile(void);

// Operates as thread opening point to handle static node bt
void handle_bt_static(void);

#endif
