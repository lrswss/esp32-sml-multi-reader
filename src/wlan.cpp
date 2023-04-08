/***************************************************************************
  Copyright (c) 2023 Lars Wessels

  This file a part of the "ESP32-SML-Multi-Reader" source code.
  https://github.com/lrswss/esp32-sml-multi-reader

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

***************************************************************************/


#include "wlan.h"
#include "utils.h"
#include "rtc.h"

WiFiManager wm;

// vTask to check Wifi connection every WIFI_CHECK_SECS
// if connection is down retry every WIFI_RETRY_SECS
static void wifiConnectionTask(void* parameter) {
    Serial.printf("%ld: Starting WiFi reconnect task with interval %d secs\n", millis(), WIFI_CHECK_SECS);
    while (1) {
        if (WiFi.status() == WL_CONNECTED) {
            switchLED(false);
            if (millis() > (WIFI_CHECK_SECS * 1000)) {
                Serial.printf("%ld: Uplink to SSID %s ready (RSSI %d dBm)\n", 
                    millis(), WiFi.SSID().c_str(), WiFi.RSSI());
            }
            timeClient.update();
        } else {
            switchLED(true);
            wifiReconnect();
        }
        printFreeStackWatermark("wificonnect_task");
        vTaskDelay((WIFI_RETRY_SECS * 1000) / portTICK_PERIOD_MS);
    }
}


// connect to local WiFi or automatically start access
// point with WiFiManager if not yet configured
void startWifi() {
    String apname = String(WIFI_AP_SSID) + "-" + systemID();
    const char* menu[] = {"wifi", "restart"};

    wm.setDebugOutput(true, "WiFi: ");  // bug in WiFiManager? autoconnect doesn't work if set to 'false'
    wm.setMinimumSignalQuality(WIFI_MIN_RSSI);
    wm.setScanDispPerc(true);
    wm.setConfigPortalTimeout(WIFI_CONFIG_TIMEOUT_SECS);
    wm.setConnectTimeout(WIFI_CONNECT_TIMEOUT_SECS);
    wm.setWebServerCallback(stopWatchdog);
    wm.setSaveConfigCallback(startWatchdog);
    wm.setMenu(menu, 2);
    switchLED(true);
    if (!wm.autoConnect(apname.c_str())) {
        Serial.printf("%ld: Failed to connect to a WiFi network...restarting!\n", millis());
        blinkLED(20, 100);
        ESP.restart();
    }
    Serial.printf("WiFi: RSSI %d dBm\n", WiFi.RSSI());
    xTaskCreate(wifiConnectionTask, "WiFi reconnect task", 2048, NULL, 2, NULL);
    blinkLED(4, 100);
}


void wifiReconnect() {
    xSemaphoreTake(SerialLock, portMAX_DELAY);
    Serial.printf("%ld: Trying to reconnect to SSID %s...", millis(), WiFi.SSID().c_str());
    if (WiFi.reconnect()) {
        Serial.println(F("OK"));
        switchLED(false);
    } else {
        Serial.println(F("failed!"));
        switchLED(true);
    }
    xSemaphoreGive(SerialLock); 
}