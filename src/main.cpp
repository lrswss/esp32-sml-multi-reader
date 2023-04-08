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

#include "config.h"
#include "smlparser.h"
#include "smlreader.h"
#include "wlan.h"
#include "mqtt.h"
#include "rtc.h"
#include "utils.h"


void setup() {
    const uint8_t SMLReaderPins[] = SML_READER_PINS;

    startWatchdog();
    pinMode(LED_PIN, OUTPUT);
    blinkLED(2, 500);

    Serial.begin(115200);
    delay(500);
    Serial.printf("\nESP32-SML-Multi-Reader (v%d)\n", FIRMWARE_VERSION);

    SerialLock = xSemaphoreCreateMutex();
    if (SerialLock == NULL)
        Serial.println(F("Failed to created mutex for Serial output!")); 

    startWifi();
    startNTPSync();
#ifdef MQTT_BROKER
    startMQTT();
#endif

	for (uint8_t i = 0; i < sizeof(SMLReaderPins); i++) {
		SMLReader *smlreader = new SMLReader(SMLReaderPins[i]);
        smlreaderList->push_back(smlreader);
        smlreader->startReader();
        smlreader->startPrinter();
    }
#ifdef DEBUG_MEMORY
    xTaskCreate(debugTask, "Debug task", 2048, NULL, 10, NULL);
#endif
}


void loop() {
    static time_t lastPublishMillis = millis();

    if ((millis() - lastPublishMillis) > (MQTT_INTERVAL_SECS * 1000)) {
        blinkLED(1, 50);
        xSemaphoreTake(SerialLock, portMAX_DELAY);
        for (std::list<SMLReader*>::iterator it = smlreaderList->begin(); it != smlreaderList->end(); ++it) {
            publishData((*it)->getReadings());
        }
        xSemaphoreGive(SerialLock);
        lastPublishMillis = millis();
    }
    esp_task_wdt_reset(); // feed the dog...
}