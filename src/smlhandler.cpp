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

#include "smlparser.h"
#include "smlhandler.h"
#include "utils.h"


void EnergyFromGridTotal(SMLDeviceReadings *data, const byte *obis) {
    smlOBISWh(data->energyFromGridTotal);
}


void EnergyToGridTotal(SMLDeviceReadings *data, const byte *obis) {
    smlOBISWh(data->energyToGridTotal);
}


void PowerFromGridTotal(SMLDeviceReadings *data, const byte *obis) {
    smlOBISW(data->powerFromGridTotal);
}


void PowerToGridTotal(SMLDeviceReadings *data, const byte *obis) {
    smlOBISW(data->powerToGridTotal);
}


void PowerFromGridL1(SMLDeviceReadings *data, const byte *obis) {
    smlOBISW(data->powerFromGridL1);
}


void PowerFromGridL2(SMLDeviceReadings *data, const byte *obis) {
    smlOBISW(data->powerFromGridL2);
}


void PowerFromGridL3(SMLDeviceReadings *data, const byte *obis) {
    smlOBISW(data->powerFromGridL3);
}

// parse 3 byte manufacturer signature
void Manufacturer(SMLDeviceReadings *data, const byte *obis) { 
    uint8_t i = 0;
    char *pos; 

    pos = (char*)memmem(data->fullMessage, data->msgSize, obis, 6);
    while (pos != NULL && i++ < 24) {
        if (*(pos + i) == 0x04) { // marker for list entry 3 byte signature
            memcpy(data->manufacturer, (pos + i + 1), 3);
            data->manufacturer[3] = '\0';
            return;
        }
    }
}


// parse serial number / server id
void Serialnumber(SMLDeviceReadings *data, const byte *obis) {
    uint8_t i = 0;
    char *pos;
    char serialnumber[10];

    pos = (char*)memmem(data->fullMessage, data->msgSize, obis, 6);
    while (pos != NULL && i++ < 24) {
        if (*(pos + i) == 0x0B) { // 0x0B marks list entry with 10 bytes
            memcpy(serialnumber, (pos + i + 1), 10);
            arr2str(serialnumber, sizeof(serialnumber), data->serialnumber);
            return;
        }
    }
}