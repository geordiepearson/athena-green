/**
 * 
 * BLE (Bluetooth Low Energy) module for project athena-green CSSE4011
 * 
 * Copyright Haoxi Tan & Geordie Pearson 2022
 */

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <sys/byteorder.h>
#include <drivers/gpio.h>
#include <stdio.h>

// #include <toolchain.h>
#include <logging/log.h>
#include "base_ble.h"

LOG_MODULE_REGISTER(ble_module, LOG_LEVEL_DBG);

#define LED1_RED DT_ALIAS(led1_red)
#define LED1_GREEN DT_ALIAS(led1_green)
#define LED1_BLUE DT_ALIAS(led1_blue)

static const struct gpio_dt_spec led1r = GPIO_DT_SPEC_GET(LED1_RED, gpios);
static const struct gpio_dt_spec led1g = GPIO_DT_SPEC_GET(LED1_GREEN, gpios);
static const struct gpio_dt_spec led1b = GPIO_DT_SPEC_GET(LED1_BLUE, gpios);


#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);


struct advert_user_data  {
    int8_t rssi;
    bt_addr_le_t *addr;
};


void led_init() {
     int retr, retg, retb;
    retr = gpio_pin_configure_dt(&led1r, GPIO_OUTPUT_ACTIVE);
    retg = gpio_pin_configure_dt(&led1g, GPIO_OUTPUT_ACTIVE);
    retb = gpio_pin_configure_dt(&led1b, GPIO_OUTPUT_ACTIVE);
    if (retr < 0) {
        LOG_ERR("failed to configure led1r gpio pin");
        return;
    }
    if (retg < 0) {
        LOG_ERR("failed to configure led1g gpio pin");
        return;
    }
    if (retb < 0) {
        LOG_ERR("failed to configure led1b gpio pinLOG_ERR");
        return;
    } 

    LOG_DBG("led init OK");

}



/**
 * @brief Callback for BLE scanning, checks weather the returned 
 *          UUID matches the custom UUID of the mobile device.
 *        If matched, attempt to connect to device.
 * 
 * The callback should return true to continue parsing, or false to stop parsing. 
 * 
 * @param data Callback data from scanning
 * @param user_data Device User data
 * @return true 
 * @return false 
 */
static bool parse_device(struct bt_data *data, void *user_data)
{
    
    struct advert_user_data *adv_user_dat = user_data;
    
    if (data->type == STATIC_ADV_TYPE)
    {
        // LOG_INF("mobile adv found, rssi: %d", adv_user_dat->rssi);
        struct static_ad sad;
        memcpy(&sad, data->data, sizeof(sad));
        LOG_PRINTK("{\"static_id\":%d, \"ttl\":%d, \"mobile_id\":%d, \"b1\":\"%c\",\"b1r\":%d,\"b2\":\"%c\",\"b2r\":%d,\"b3\":\"%c\",\"b3r\":%d,\"speed\":%d,\"direction\":%d,\"uptime\":%d}\n", sad.static_id, sad.ttl,
                sad.m_ad.m_id, sad.m_ad.b1_id, sad.m_ad.b1_rssi, 
                sad.m_ad.b2_id, sad.m_ad.b2_rssi,
                sad.m_ad.b3_id, sad.m_ad.b3_rssi, sad.m_ad.speed, sad.m_ad.direction, k_uptime_get_32());
        return false;
        
    }

    if (data->type == MOBILE_ADV_TYPE) {
        struct mobile_ad mad;
        memcpy(&mad, data->data, sizeof(mad));
        LOG_PRINTK("{\"mobile_id\":%d, \"b1\":\"%c\",\"b1r\":%d,\"b2\":\"%c\",\"b2r\":%d,\"b3\":\"%c\",\"b3r\":%d,\"speed\":%d,\"direction\":%d,\"uptime\":%d,}\n",
                mad.m_id, mad.b1_id, mad.b1_rssi, 
                mad.b2_id, mad.b2_rssi,
                mad.b3_id, mad.b3_rssi, mad.speed,mad.direction,k_uptime_get_32());
        return false;
    }
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


    // LOG_INF("some device found");
    // bt_data_parse(ad, parse_device, (void *)addr);
    struct advert_user_data user_data = {
        .rssi = rssi,
        .addr = addr
    };
    bt_data_parse(ad, parse_device, &user_data);

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
        LOG_ERR("Scanning failed to start (err %d)\n", err);
        return;
    }

    // LOG_INF("Scanning successfully started\n");
}



/**
 * @brief BLE Base entry thread, starts initial ble scanning.
 *          When a valid mobile device is connected.
 */
void thread_ble_base(void)
{
    int err,ret;

    err = bt_enable(NULL);



    if (!device_is_ready(led.port)) {
        LOG_ERR("Bluetooth init failed, LED0 not ready\n");
    }

    gpio_pin_set_dt(&led, 1);

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
        if (ret < 0) {
            LOG_ERR("failed to configure led0 gpio pin\n");
            return;
        }

    if (err)
    {
        LOG_ERR("Bluetooth init failed (err %d)\n", err);
        return;
    }

    LOG_INF("Bluetooth initialized\n");

    
  
    while (1) {
        start_scan();
        k_msleep(300);
        bt_le_scan_stop();
        
    }

    

    //Should not reach here
    // LOG_INF("Debug_1\n");
}


