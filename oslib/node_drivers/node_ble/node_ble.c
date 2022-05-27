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

/* states */
#define SCANNING 0
#define ADVERTISING 1

#define TOO_CLOSE_RSSI -62

uint32_t last_switchtime = 0;
bool time_corrected = false;
bool state = SCANNING;
uint32_t last_too_close = 0;
bool too_close = false;

struct advert_user_data {
	int8_t rssi;
	const bt_addr_le_t *addr;
};

bool adv_found = false;


#ifndef MOBILE_NODE
/**
 * these two flags are for the static node to track if they have picked up 
 * anything during scanning phase to relay during advertising phase
 **/
bool mobile_adv_found = false;
bool static_adv_found = false;
// true for scanning for mobile ads, otherwise for scanning for static ads
bool is_turn_for_mobile_ads = false; 
#endif

bool is_advertising = false;
bool is_scanning = false;

// might have a few found m ads?
struct mobile_ad found_m_adv;
struct static_ad found_s_adv;

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
    
#if MOBILE_NODE == 1
    if (data->type == MOBILE_ADV_TYPE)
    {
        printk("mobile adv found, rssi: %d\n", adv_user_dat->rssi);
        // time synchonrization: when we find a packet, we switch to scanning mode?
        adv_found = true;

        // detect if it is too close
        if (adv_user_dat->rssi > TOO_CLOSE_RSSI) {
        	too_close = true;
        	last_too_close = k_uptime_get_32();
        }

        return false;
        
    }
#else
    // STATIC NODE ONLY CODE

    // XXX: prioritize static adv?
    if (is_turn_for_mobile_ads) {
    	if (data->type == MOBILE_ADV_TYPE) {
	    	printk("mobile adv found by SN %d\n", adv_user_dat->rssi);
	        // time synchonrization: when we find a packet, we switch to scanning mode?
	        mobile_adv_found = true;

	        // store it in the found adv
	        memcpy(&found_m_adv, data->data, sizeof(found_m_adv));
	        printk("m_id: %02x, b1 %c b1r %d b2 %c b2r %d b3 %c b3r %d\n", found_m_adv.m_id,
	        	found_m_adv.b1_id, found_m_adv.b1_rssi, found_m_adv.b2_id, found_m_adv.b2_rssi,
	        	found_m_adv.b3_id, found_m_adv.b3_rssi);
	        return false;
	    }

    }

    if (data -> type == STATIC_ADV_TYPE) {
    	printk("static adv found by SN %d\n", adv_user_dat->rssi);
    	
    	
    	if ( ((struct static_ad*) data->data)->static_id != M_ID && 
    			((struct static_ad*) data->data)->ttl > 1) { // do not relay my own packet
    		memcpy(&found_s_adv, data->data, sizeof(found_s_adv));
    		// dont forward dead packets
    		static_adv_found = true;
    		found_s_adv.ttl -= 1;
	    	printk("ttl: %d, s_id: %02x m_id: %02x, b1 %c b1r %d b2 %c b2r %d b3 %c b3r %d\n", found_s_adv.ttl, found_s_adv.static_id,
	    		found_s_adv.m_ad.m_id, found_s_adv.m_ad.b1_id, found_s_adv.m_ad.b1_rssi, 
	    		found_s_adv.m_ad.b2_id, found_s_adv.m_ad.b2_rssi,
	        	found_s_adv.m_ad.b3_id, found_s_adv.m_ad.b3_rssi);
	    	return false;
    	} else {
    		printk("static adv came from me: %02x ttl %02x\n", ((struct static_ad*) data->data)->static_id, 
    			((struct static_ad*) data->data)->ttl);
    		// return false;
    	}
    	
    }

    


#endif

    return true;
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
// green for mobile node
#define LED1_NODE DT_ALIAS(led1)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED1_NODE, gpios);

#else

// led0 (blue) for static node
#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

#endif

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

	gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);

	while (1) {

		// printk("uptim32: %d\n", k_uptime_get_32());
		// last_switchtime = k_uptime_get_32();
		// printk("last_switchtime: %d\n", k_uptime_get_32());

		// k_msleep(100);

		/*** state switcher ***/
		if (state == SCANNING && ((k_uptime_get_32() - last_switchtime) > 200)) {
			printk("[%d] Switching to advertising\n", k_uptime_get_32());
			
			state = ADVERTISING;
			last_switchtime = k_uptime_get_32();
			continue;
		} else if (state == ADVERTISING && ((k_uptime_get_32() - last_switchtime) > 50)) {
			printk("[%d] Switching to scanning\n", k_uptime_get_32());
			state = SCANNING;
			last_switchtime = k_uptime_get_32();
			continue;
		}

		if (too_close == true && (k_uptime_get_32() - last_too_close > 1500)) {
			too_close = false;
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
				// k_msleep(200);
			}

			

			gpio_pin_set_dt(&led, 1);

		} else if (state == SCANNING) {
			

			// make timing a bit more random
			// if (k_uptime_get_32() % 2 == 1) {
			// 	k_msleep(30);
			// }

			if (adv_found == true && time_corrected == false) { // only do this once
				printk("adv found, staying in scanning mode a little longer\n");
				last_switchtime = k_uptime_get_32();
				adv_found = false;
				time_corrected = true;
			}

			is_advertising = false;
			if (is_scanning == false) {
				bt_le_adv_stop();
				printk("[%d] Adv stopped\n", k_uptime_get_32());
				
				bt_le_scan_stop();
				// k_msleep(100);

				adv_found = false;
				start_scan(); // this will set is_scanning to true if success
				printk("[%d] Scanning started\n", k_uptime_get_32());
		
			}

			if (!too_close) {
				gpio_pin_set_dt(&led, 0); // if they are too close, hold LED on
			}
			
		}

		k_msleep(50);

	}
}


#ifndef MOBILE_NODE

/**
 * Bluetooth code for static nodes - creates a "mesh network" by forwarding mobile node packets to other static nodes
 **/
void handle_bt_static(void) {

	printk("static node.\n");
	init_bt();

	gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);

	int ret;
	// listen & adv etc..
	while (1) {

		// printk("uptim32: %d\n", k_uptime_get_32());
		// last_switchtime = k_uptime_get_32();
		// printk("last_switchtime: %d\n", k_uptime_get_32());

		// k_msleep(100);

		/*** state switcher ***/
		if (state == SCANNING && ((k_uptime_get_32() - last_switchtime) > 400)) {
			printk("[%d] Switching to advertising\n", k_uptime_get_32());
			
			state = ADVERTISING;
			last_switchtime = k_uptime_get_32();
			continue;
		} else if (state == ADVERTISING && ((k_uptime_get_32() - last_switchtime) > 250)) {
			printk("[%d] Switching to scanning (mobileturn:%i)\n", k_uptime_get_32(), is_turn_for_mobile_ads);
			state = SCANNING;
			last_switchtime = k_uptime_get_32();
			continue;
		}

		if (state == SCANNING) {

			is_turn_for_mobile_ads = ! is_turn_for_mobile_ads; // flip this

			
			is_advertising = false;
			if (is_scanning == false) {
				bt_le_adv_stop();
				printk("[%d] SN Adv stopped\n", k_uptime_get_32());
				
				bt_le_scan_stop();
				// k_msleep(100);

				adv_found = false;
				start_scan(); // this will set is_scanning to true if success
				printk("[%d] SN Scanning started\n", k_uptime_get_32());
			
			}
			gpio_pin_set_dt(&led, 0); 
		} else if (state == ADVERTISING) {
			// stop scanning and start advertising
			
			// printk("[%d] Scanning stopped \n", k_uptime_get_32());

			// k_msleep(30);

			// XXX need to advertise both found mobile adverts and relay static adverts
			if (is_advertising == false) { // only start advertising when it isn started already

				is_advertising = true;

				printk("about to start adv staticfound:%i mobilefound:%i\n", static_adv_found, mobile_adv_found);

				if (static_adv_found) {

					struct bt_data data_ad[] = {
							BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
							BT_DATA(STATIC_ADV_TYPE, &found_s_adv, sizeof(found_s_adv))
							// BT_DATA(BT_DATA_UUID128_ALL,)
					};

					bt_le_adv_stop();
					bt_le_scan_stop();
					is_scanning = false;
					
					ret = bt_le_adv_start(BT_LE_ADV_CONN_NAME, data_ad, ARRAY_SIZE(data_ad), NULL, 0);
					printk("[%d] SN Adv started, turn for mobile:%i ret:%d.\n", k_uptime_get_32(), is_turn_for_mobile_ads, ret);
					if (ret) {
						printk("SN Advertising failed with code %d.\n", ret);
						// return;
					}
					is_advertising = true;

				} else if (mobile_adv_found) {
					struct static_ad s_ad = {.ttl=4, .static_id = M_ID, .m_ad = found_m_adv};

					struct bt_data data_ad[] = {
							BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
							BT_DATA(STATIC_ADV_TYPE, &s_ad, sizeof(s_ad))
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
					
				}

				
			}
			
			gpio_pin_set_dt(&led, 1);
			// reset at the end of starting adv
			mobile_adv_found = false;
			static_adv_found = false;

		}

		k_msleep(50);
	}

	
}

#endif