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

#include "utils.h"
#include "rtc.h"
#include "config.h"

SemaphoreHandle_t SerialLock;


// turn array of given length into a null-terminated hex string
void arr2str(const char *arr, int len, char *buf) {
    for (int i = 0; i < len; i++)
        sprintf(buf + i * 2, "%02X", arr[i]);
}


// blink LED
void blinkLED(uint8_t repeat, uint16_t pause) {
    for (uint8_t i = 0; i < repeat; i++) {
        digitalWrite(LED_PIN, LOW);
        delay(pause);
        digitalWrite(LED_PIN, HIGH);
        if (repeat > 1)
        delay(pause);
    }
}


// for debugging stack size of tasks
void printFreeStackWatermark(const char *taskName) {
#ifdef DEBUG_MEMORY
    Serial.printf("[DEBUG] %s: StackHighWaterMark: %d\n", taskName, uxTaskGetStackHighWaterMark(NULL));
#endif
}


// turn LED on or off
void switchLED(bool state) {
    digitalWrite(LED_PIN, !state ? HIGH : LOW);
}


// returns given string without spaces
char* removeSpaces(char *str) {
    uint8_t i = 0, j = 0;
    while (str[i++]) {
        if (str[i-1] != ' ')
        str[j++] = str[i-1];
    }
    str[j] = '\0';
    return str;
}


// returns hardware system id (ESP's chip id)
String systemID() {
    uint8_t mac[6];
    char sysid[7];
    
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    sprintf(sysid, "%02X%02X%02X", mac[3], mac[4], mac[5]);
    return String(sysid);
}


// initialize and start watchdog for this thread
void startWatchdog() {
    esp_task_wdt_init(WATCHDOG_TIMEOUT_SEC, true); 
    esp_task_wdt_add(NULL);
}


// stop watchdog timer
void stopWatchdog() {
    esp_task_wdt_delete(NULL);
    esp_task_wdt_deinit();
}


void debugTask(void* parameter) {
    while (1) {
        xSemaphoreTake(SerialLock, portMAX_DELAY);
        Serial.printf("[DEBUG] runtime: %s, free heap: %d\n", getRuntime(), ESP.getFreeHeap());
        printFreeStackWatermark("debug_task");
        xSemaphoreGive(SerialLock); 
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}