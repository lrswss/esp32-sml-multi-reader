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

#ifndef _SMLHANDLER_H
#define _SMLHANDLER_H

#include <Arduino.h>

void Manufacturer(SMLDeviceReadings *data, const byte *obis);
void Serialnumber(SMLDeviceReadings *data, const byte *obis);
void EnergyFromGridTotal(SMLDeviceReadings *data, const byte *obis);
void EnergyToGridTotal(SMLDeviceReadings *data, const byte *obis);
void PowerFromGridTotal(SMLDeviceReadings *data, const byte *obis);
void PowerToGridTotal(SMLDeviceReadings *data, const byte *obis);
void PowerFromGridL1(SMLDeviceReadings *data, const byte *obis);
void PowerFromGridL2(SMLDeviceReadings *data, const byte *obis);
void PowerFromGridL3(SMLDeviceReadings *data, const byte *obis);

#endif