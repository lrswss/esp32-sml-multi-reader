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

#include "smlreader.h"
#include "smlparser.h"
#include "utils.h"
#include "testdata.h"
#include "config.h"


std::list<SMLReader *> *smlreaderList = new std::list<SMLReader *>();


SMLReader::SMLReader(const uint8_t pin) {
    this->begin(pin);
    resetSMLReadings(&this->readings);
}


bool SMLReader::begin(const uint8_t pin) {
    if (pin >= 0 && pin <= 36) {  // ESP32
        this->readings.pin = pin;
        this->ss = std::unique_ptr<SoftwareSerial>(new SoftwareSerial());
        this->ss->begin(9600, SWSERIAL_8N1, this->readings.pin, -1, false, SML_MSG_BUFFER);
        this->ss->enableTx(false);
        this->ss->enableRx(true);
        return true;
    } else {
        Serial.println(F("SMLReader(): invalid pin number!"));
        return false;
    }
}

#ifndef DEBUG_TESTDATA
void SMLReader::read() {
    while (this->ss->available()) {
        if (readSMLByte(this->ss->read(), &readings)) {
            this->ss->flush();
            return;
        }
    }
}

#else
// feed data from testdata.h
void SMLReader::read() {
    static uint8_t count = 0;
    static uint8_t* data;

    if (count >= sizeof(SML_TESTDATA_SIZE)/2)
        count = 0;
    data = SML_TESTDATA[count];

    for (uint16_t i = 0; i < SML_TESTDATA_SIZE[count]; i++) {
        if (readSMLByte(*data, &readings)) {
            count++;
            return;
        }
        data++;
    }
}
#endif


SMLDeviceReadings SMLReader::getReadings() {
    time_t time_utc;
    static SMLDeviceReadings readings;

    readings = this->readings;

    time(&time_utc); // eventually expire manufacturer on pin to mark missing SML readings
    if (time_utc - readings.timestamp > (SML_DATA_EXPIRE_SECS * 3))
        memset(readings.manufacturer, 0, sizeof(readings.manufacturer));

    if (strlen(readings.serialnumber) == 0)
        snprintf(readings.serialnumber, 20, "%.20d", 0);
    else
        readings.serialnumber[20] = '\0';

    return readings;
}


void SMLReader::printReadings() {
    xSemaphoreTake(SerialLock, portMAX_DELAY);
    Serial.printf("%ld: SMLReader (Pin %d)\n", millis(), this->readings.pin);
    printSMLReadings(this->getReadings());
    xSemaphoreGive(SerialLock); 
}


void SMLReader::readingTask() {
    time_t last = 0;
    vTaskDelay(100/portTICK_PERIOD_MS);
    Serial.printf("%ld: Starting SMLReader task for pin %d\n", millis(), this->readings.pin);
    while(1) {
        this->read();
        if ((millis() - last) > 5000) {
            last = millis();
            printFreeStackWatermark("smlreader_task");
        }
        vTaskDelay(random(SML_READER_INTERVAL_MS-500,SML_READER_INTERVAL_MS+500)/portTICK_PERIOD_MS);
    }
}


void SMLReader::printerTask() {
    vTaskDelay(2000/portTICK_PERIOD_MS);
    while(1) {
        this->printReadings();
        printFreeStackWatermark("smlprinter_task");
        vTaskDelay((SML_PRINT_INTERVAL_SECS * 1000)/portTICK_PERIOD_MS);
    }
}


void SMLReader::printerTaskWrapper(void* _this) {
    static_cast<SMLReader*>(_this)->printerTask();
}


void SMLReader::startPrinter() {
    char taskName[48];
    sprintf(taskName, "SMLReader print data task (Pin %d)", this->readings.pin);
    xTaskCreate(this->printerTaskWrapper, taskName, 3072, this, 1, NULL);
    delay(100);
}


void SMLReader::readingTaskWrapper(void* _this) {
    static_cast<SMLReader*>(_this)->readingTask();
}


void SMLReader::startReader() {
    char taskName[48];
    sprintf(taskName, "SMLReader serial read task (Pin %d)", this->readings.pin);
    xTaskCreate(this->readingTaskWrapper, taskName, 2048, this, 5, NULL);
    delay(100);
}
