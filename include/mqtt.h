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

#ifndef _MQTT_H
#define _MQTT_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include "smlparser.h"
#include "config.h"

#define MQTT_CHECK_SECS 15
#define MQTT_KEEPALIVE_SECS MQTT_INTERVAL_SECS*1.5
#define MQTT_CLIENT_ID "smlreader_%d"
#define MQTT_CONNECT_WAIT_SECS 10

#if defined(MQTT_TLS) && MQTT_BROKER_PORT == 1883
#undef MQTT_BROKER_PORT
#define MQTT_BROKER_PORT 8883
#endif

void startMQTT();
void publishData(const SMLDeviceReadings &data);

#endif