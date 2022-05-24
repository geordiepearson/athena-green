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

struct advert_user_data {
	int8_t rssi;
	bt_addr_le_t *addr;
};

static bool ble_connected = false;
static bool ble_advertising = false;

/**
 * Enables bluetooth functionality for node.
 **/
void init_bt(void) {
	int ret;
	ret = bt_enable(NULL);
	if (ret) {
		printk("Bluetooth init failed with code %d.\n", ret);
		return;	
	}
}

/**
 * Bluetooth code for static node. Just advertises the infrared sensor data,
 * because the RSSI information is based on where it is sending anyway.
 *
 **/
/*void handle_bt_static(void) {

	printk("static node.\n");
	init_bt();
	
	struct bt_data data_ad[] = {
				BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
				BT_DATA(BT_CUSTOM_DATA, 0, 2)
				};
	int ret = bt_le_adv_start(BT_LE_ADV_CONN_NAME, data_ad, ARRAY_SIZE(data_ad), NULL, 0);
	if (ret) {
		printk("Advertising failed with code %d.\n", ret);
	}
	printk("Advertising started.\n");
	
	while(1) {
		uint8_t data[2] = {(ranging_bufs[0].distance_cm & 0xFF00) >> 8,
			ranging_bufs[0].distance_cm & 0x00FF};
		struct bt_data new_ad[] = {
				BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
				BT_DATA(BT_CUSTOM_DATA, data, 2)
				};
		ret = bt_le_adv_update_data(new_ad, 2, NULL, 0); 
		k_msleep(500);
	}	
}
*/
