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

#include <drivers/gpio.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <sys/byteorder.h>

#include "node_ble.h"

struct advert_user_data {
	int8_t rssi;
	const bt_addr_le_t *addr;
};

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

bool is_advertising = false;
bool is_scanning = false;

int8_t beacon_strengths [3] = {0xff,};

// static bool ble_advertising = false;

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
 * @brief Callback for BLE scanning, checks weather the returned 
 *          UUID matches the custom UUID of the mobile device.
 *        If matched, attempt to connect to device.
 * 
 * @param data Callback data from scanning
 * @param user_data Device User data
 * @return true to continue scanning
 * @return false to stop scanning?
 */
static bool parse_device(struct bt_data *data, void *user_data)
{
    
    struct advert_user_data *adv_user_dat = user_data;
    
    if (data->type == MOBILE_ADV_TYPE)
    {
        printk("mobile adv found, rssi: %d\n", adv_user_dat->rssi);
        return false;
        
    }
    return false;
}



/**
 * @brief Callback function for when scan detects device, scanned devices
 *          are filtered by their connectibilty and scan data is parsed.
 * 
 * @param addr Device Address
 * @param rssi RSSI 
 * @param type Device Type
 * @param ad Adv Data
 */
static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
                         struct net_buf_simple *ad)
{

    /* We're only interested in connectable events */
    if (type == BT_GAP_ADV_TYPE_ADV_IND ||
        type == BT_GAP_ADV_TYPE_ADV_DIRECT_IND)
    {
        // LOG_INF("some device found");
        // bt_data_parse(ad, parse_device, (void *)addr);
        struct advert_user_data user_data = {
            .rssi = rssi,
            .addr = addr
        };
        bt_data_parse(ad, parse_device, &user_data);
    }
}

/**
 * @brief Starts passive BLE scanning for nearby
 *          devices.
 */
static void start_scan(void)
{
    int err;

    err = bt_le_scan_start(BT_LE_SCAN_ACTIVE, device_found);
    if (err)
    {
        printk("Scanning failed to start (err %d)\n", err);
        return;
    }

    printk("Scanning successfully started\n");
    is_scanning = true;
}



#if MOBILE_NODE == 1

/* flash LED for debugging and distancing alert */
#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);


/* states */
#define SCANNING 0
#define ADVERTISING 1

uint32_t last_switchtime = 0;
bool state = SCANNING;

/**
 * mobile bluetooth thread
 * - broadcasts RSSI of surrounding ibeacons and sensor node
 * - scans for nearly RSSI of ibeacons and other mobile nodes
 */
void handle_bt_mobile(void) {
	int ret;
	ret = bt_enable(NULL);
	if (ret) {
		printk("Bluetooth init failed with code %d.\n", ret);
		return;	
	}

	while (1) {

		// printk("uptim32: %d\n", k_uptime_get_32());
		// last_switchtime = k_uptime_get_32();
		// printk("last_switchtime: %d\n", k_uptime_get_32());

		// k_msleep(100);

		/*** state switcher ***/
		if (state == SCANNING && ((k_uptime_get_32() - last_switchtime) > 500)) {
			printk("[%d] Switching to advertising\n", k_uptime_get_32());
			
			state = ADVERTISING;
			last_switchtime = k_uptime_get_32();
			continue;
		} else if (state == ADVERTISING && ((k_uptime_get_32() - last_switchtime) > 500)) {
			printk("[%d] Switching to scanning\n", k_uptime_get_32());
			state = SCANNING;
			last_switchtime = k_uptime_get_32();
			continue;
		}

		
		if (state == ADVERTISING) {
			// stop scanning and start advertising
			
			// printk("[%d] Scanning stopped \n", k_uptime_get_32());

			// k_msleep(30);

			if (is_advertising == false) { // only start advertising when it isn started already
				struct mobile_ad m_ad = {.m_id = M_ID, .b1_id = 'X', .b1_rssi = 99, .b2_id = 'Y', .b2_rssi = 99, .b3_id = 'Z', .b3_rssi = 99, .speed=66, .direction=1};

				struct bt_data data_ad[] = {
						BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
						BT_DATA(MOBILE_ADV_TYPE, &m_ad, sizeof(m_ad))
						// BT_DATA(BT_DATA_UUID128_ALL,)
				};

				bt_le_adv_stop();
				bt_le_scan_stop();
				is_scanning = false;
				
				ret = bt_le_adv_start(BT_LE_ADV_CONN_NAME, data_ad, ARRAY_SIZE(data_ad), NULL, 0);
				printk("[%d] Adv started %d.\n", k_uptime_get_32(), ret);
				if (ret) {
					printk("Advertising failed with code %d.\n", ret);
					// return;
				}
				is_advertising = true;
				k_msleep(200);
			}

			

			gpio_pin_set_dt(&led, 1);

		} else if (state == SCANNING) {
			

			// make timing a bit more random
			// if (k_uptime_get_32() % 2 == 1) {
			// 	k_msleep(30);
			// }

			is_advertising = false;
			if (is_scanning == false) {
				bt_le_adv_stop();
				printk("[%d] Adv stopped\n", k_uptime_get_32());
				
				bt_le_scan_stop();
				// k_msleep(100);

				start_scan(); // this will set is_scanning to true if success
				printk("[%d] Scanning started\n", k_uptime_get_32());
		
			}

			gpio_pin_set_dt(&led, 0);
			
		}

		k_msleep(50);

	}
}

#endif

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
