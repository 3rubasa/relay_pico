/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include <iostream>
#include <sstream>
#include <memory>

#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/mqtt_priv.h"
#include "tusb.h"

#include "lwip/apps/httpd.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwipopts.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "SonoffMiniR2Detector.h"

#include "lwip/apps/httpd.h"
#include "lwipopts.h"

#include "ssi.h"

#include "creds.h"

#define DEBUG_printf printf

#define SONOFF_MINIR2_IP_ADDRESS "192.168.100.19"
#define SONOFF_MINIR2_CONNECT_TIMEOUT 2
#define SONOFF_MINIR2_CONNECT_TRIES 2

#define RELAY_CONTROL_PIN 28

bool g_lastState = false;

int main() {
    stdio_init_all();
    gpio_init(RELAY_CONTROL_PIN);
    gpio_set_dir(RELAY_CONTROL_PIN, GPIO_OUT);
    gpio_put(RELAY_CONTROL_PIN, 0);
    int err = 0;

    if ((err = cyw43_arch_init()) != 0) {
        printf("Error while initializing cyw43: %d\n", err);
        return 1;
    }

    cyw43_arch_enable_sta_mode();

    // Connect to the WiFI network - loop until connected
    // WIFI_SSID and WIFI_PASSWORD come from creds.h
    
    while((err = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) != 0){
        printf("Error while connecting: %d\n", err);
        if (err == PICO_ERROR_TIMEOUT) {
            printf("Probably SSID specified in creds.h %s was not found: \n", WIFI_SSID);
        } else if (err == PICO_ERROR_BADAUTH) {
            printf("Probably password specified in creds.h %s is incorrect: \n", WIFI_PASSWORD);
        }
        printf("Attempting to connect...\n");
    }

    // Print a success message once connected
    printf("WIFI Connected! \n");

    // Initialise web server
    httpd_init();
    printf("Http server initialised\n");

    // Configure SSI and CGI handler
    ssi_init(&g_lastState); 
    printf("SSI Handler initialised\n");

    int tryNum = 0;
    while(true) {
        try {
            SonoffMiniR2Detector detector(SONOFF_MINIR2_IP_ADDRESS);
            if (detector.Detect(SONOFF_MINIR2_CONNECT_TIMEOUT))
            {
                tryNum=0;
                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
                gpio_put(RELAY_CONTROL_PIN, 1);
                g_lastState = true;
            } else {
                tryNum++;
                if (tryNum == SONOFF_MINIR2_CONNECT_TRIES) {
                    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
                    gpio_put(RELAY_CONTROL_PIN, 0);
                    g_lastState = false;
                    tryNum = 0;
                }
            }
        }
        catch (const std::runtime_error& ex) {
            DEBUG_printf("Main Loop: Exception: %s\n", ex.what());
                
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            gpio_put(RELAY_CONTROL_PIN, 0);
            g_lastState = false;
        }

        sleep_ms(1000);
    }
}