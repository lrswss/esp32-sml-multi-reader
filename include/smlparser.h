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

#ifndef _SMLPARSER_H
#define _SMLPARSER_H

#include <Arduino.h>
#include <sml.h>
#include "config.h"

typedef struct {
    uint8_t pin;
    unsigned char manufacturer[4]; // 3 byte manufacturer signature
    char serialnumber[21]; // 10 bytes stored as hex string
    double energyFromGridTotal;
    double energyToGridTotal;
    double powerFromGridTotal;
    double powerToGridTotal;
    double powerFromGridL1;
    double powerFromGridL2;
    double powerFromGridL3;
    time_t timestamp; // set to '0' to invalidate dataset
    char fullMessage[SML_MSG_BUFFER];
    uint16_t msgSize;
    sml_states_t state;
} SMLDeviceReadings;

typedef struct {
    const byte OBIS[6];
    void (*Handler)(SMLDeviceReadings *data, const byte *obis);
} OBISHandler;

bool readSMLByte(byte c, SMLDeviceReadings *data);
void resetSMLReadings(SMLDeviceReadings *data);
void printSMLReadings(const SMLDeviceReadings &data);

#endif