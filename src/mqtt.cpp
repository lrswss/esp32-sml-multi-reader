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

#include "mqtt.h"
#include "smlparser.h"
#include "smlreader.h"
#include "wlan.h"
#include "utils.h"
#include "rtc.h"

static WiFiClient espClient;
static WiFiClientSecure espClientSecure;
static PubSubClient *mqtt = NULL;


// publish JSON on given MQTT topic
static void publishJSON(JsonDocument& json, char *topic, bool retain) {
#ifdef DEBUG_SML
    static char buf[1280];
#else
    static char buf[512];
#endif
    size_t bytes;

    Serial.print(millis());
    if (mqtt == NULL || !mqtt->connected() || WiFi.status() != WL_CONNECTED) {
        Serial.printf(": MQTT %s aborted, no MQTT or WiFi uplink!\n", topic);
        return;
    }

    memset(buf, 0, sizeof(buf));
    bytes = serializeJson(json, buf, sizeof(buf)-1);
    if (json.overflowed()) {
        Serial.printf(": MQTT %s aborted, JSON overflow (%d bytes)!\n", topic, bytes);
    } else {
        if (mqtt->publish(topic, buf, retain)) {
            Serial.printf(": MQTT %s %s\n", topic, buf);
        } else {
            Serial.printf(": MQTT %s failed (%d bytes)!\n", topic, bytes);
        }
    }
    json.clear();
    delay(100);
}


// publish data on base topic as JSON
void publishData(const SMLDeviceReadings &data) {
    static uint32_t lastUpdate = 0;
    char topicStr[128];
#ifdef DEBUG_SML
    StaticJsonDocument<1024> JSON;
    static char smlmsg[1024];
#else
    StaticJsonDocument<256> JSON;
#endif
    time_t time_utc;

    time(&time_utc);
    JSON.clear();
    if (!lastUpdate || (millis() - lastUpdate) > (MQTT_KEEPALIVE_SECS * 1000)) {
        if (!lastUpdate)
            JSON["msgtype"] = "startup";
        else
            JSON["msgtype"] = "keealive";
        JSON["timestamp"] = time_utc;
        JSON["uptime"] = removeSpaces(getRuntime());
#ifdef DEBUG_MEMORY
        JSON["heap"] = ESP.getFreeHeap();
#endif
        lastUpdate = millis();
        snprintf(topicStr, sizeof(topicStr), "%s/%s/state", MQTT_BASE_TOPIC, systemID().c_str());
        publishJSON(JSON, topicStr, false);
    }

    if (time_utc - data.timestamp > SML_DATA_EXPIRE_SECS) {
        if (strlen((char*)data.manufacturer))
            Serial.printf("%ld: Skipping MQTT update for %s/%s (pin %d), no recent data\n", 
                millis(), data.manufacturer, data.serialnumber, data.pin);
        else
            Serial.printf("%ld: Skipping MQTT update (pin %d), no data\n", millis(), data.pin);
        return;
    
    } else if (data.state == SML_CHECKSUM_ERROR) {
        lastUpdate = millis();
        JSON["msgtype"] = "error";
        JSON["timestamp"] = data.timestamp;
        JSON["error"] = "checksum";
        JSON["version"] = FIRMWARE_VERSION;

    } else if (data.state == SML_END) {
        lastUpdate = millis();
        JSON["msgtype"] = "error";
        JSON["timestamp"] = data.timestamp;
        JSON["error"] = "buffer";
        JSON["version"] = FIRMWARE_VERSION;

    } else {
        lastUpdate = millis();
        JSON["msgtype"] = "data";
        JSON["timestamp"] = data.timestamp;

        JSON["manufacturer"] = data.manufacturer;
        JSON["serialnumber"] = data.serialnumber;
#ifndef DEBUG_SML
        if (data.energyFromGridTotal > LONG_MIN)
            JSON["energyFromGridTotalkWh"] = data.energyFromGridTotal/1000;
        if (data.energyToGridTotal > LONG_MIN)
            JSON["energyToGridTotalkWh"] = data.energyToGridTotal/1000;
        if (data.powerFromGridTotal > LONG_MIN)
            JSON["powerFromGridTotalW"] = data.powerFromGridTotal;
        if (data.powerToGridTotal > LONG_MIN)
            JSON["powerToGridTotalW"] = data.powerToGridTotal;            
        if (data.powerFromGridL1 > LONG_MIN)
            JSON["powerFromGridL1W"] = data.powerFromGridL1;
        if (data.powerFromGridL2 > LONG_MIN)
            JSON["powerFromGridL2W"] = data.powerFromGridL2;
        if (data.powerFromGridL3 > LONG_MIN)
            JSON["powerFromGridL3W"] = data.powerFromGridL3;
        JSON["version"] = FIRMWARE_VERSION;
#else
        memset(smlmsg, 0, sizeof(smlmsg));
        arr2str(data.fullMessage, data.msgSize, smlmsg);
        JSON["sml"] = smlmsg;
#endif
        snprintf(topicStr, sizeof(topicStr), "%s/%s/%d/state",
            MQTT_BASE_TOPIC, systemID().c_str(), data.pin);
        publishJSON(JSON, topicStr, false);
    }
}


// vTask to keep connection to MQTT server (with changing id on every attempt)
static void mqttConnectionTask(void* parameter) {
    static char clientid[32];

    Serial.printf("%ld: Starting MQTT connection task with interval %d secs\n", millis(), MQTT_CHECK_SECS);
    while (1) {
        xSemaphoreTake(SerialLock, portMAX_DELAY);
        if (!WiFi.isConnected()) {
            Serial.printf("%ld: WiFi not available, cannot connect to MQTT broker %s\n", millis(), MQTT_BROKER);
            wifiReconnect();
        } else if (mqtt->connected()) {
            Serial.printf("%ld: Connection to MQTT broker %s ready\n", millis(), MQTT_BROKER);
        } else {
            snprintf(clientid, sizeof(clientid), MQTT_CLIENT_ID, (int)random(0xfffff));
            Serial.printf("%ld: Connecting to MQTT broker %s", millis(), MQTT_BROKER);
    #if defined(MQTT_USERNAME) && defined(MQTT_PASSWORD)
                Serial.printf(" with username %s", MQTT_USERNAME);
    #endif
            Serial.printf(" on port %d...", MQTT_BROKER_PORT);
    #if defined(MQTT_USERNAME) && defined(MQTT_PASSWORD)
            if (mqtt->connect(clientid, MQTT_USERNAME, MQTT_PASSWORD)) {
                Serial.println(F("OK"));
            } else {
    #else
            if (mqtt->connect(clientid)) {
                Serial.println(F("OK"));
            } else {
    #endif
                Serial.printf("failed (error %d)\n", mqtt->state());
                blinkLED(2, 50);
            }
        }
        xSemaphoreGive(SerialLock); 
        printFreeStackWatermark("mqttconnect_task");
        vTaskDelay((MQTT_CHECK_SECS * 1000) / portTICK_PERIOD_MS);
    }
}


void startMQTT() {
    uint16_t timeout = 0;
#ifdef MQTT_TLS
    espClientSecure.setInsecure();
    espClientSecure.setTimeout(MQTT_KEEPALIVE_SECS);
    mqtt = new PubSubClient(espClientSecure);
#else
    mqtt = new PubSubClient(espClient);
#endif
    mqtt->setServer(MQTT_BROKER, MQTT_BROKER_PORT);
#ifdef DEBUG_SML
    mqtt->setBufferSize(1024);
#else
    mqtt->setBufferSize(640);
#endif
    mqtt->setSocketTimeout(2); // avoid blocking
    mqtt->setKeepAlive(MQTT_KEEPALIVE_SECS);

#ifdef MQTT_TLS
    xTaskCreate(mqttConnectionTask, "MQTT reconnect task", 3840, NULL, 2, NULL);
#else
    xTaskCreate(mqttConnectionTask, "MQTT reconnect task", 2048, NULL, 2, NULL);
#endif
    while (!mqtt->connected() && timeout++ < MQTT_CONNECT_WAIT_SECS*2)
        delay(500);
}