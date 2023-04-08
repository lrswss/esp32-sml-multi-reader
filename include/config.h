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

#ifndef _CONFIG_H
#define _CONFIG_H

// Specify one or more pins (max. 6) connected to RX pin of IR reading 
// head The number of reading heads can be increased to 8 or even more
// if SML_MSG_BUFFER is reduced to about 256 bytes (depends on smart 
// meter message size) or if TLS for MQTT is disabled
#define SML_READER_PINS { 4, 13, 14, 16, 17, 21 }

// Buffer size for SML messages received with IR sensor; might need to be
// increased to about 512 bytes or even more if a smart meter sends larger 
// messages (look for "buffer exceeded message" in serial output). For 
// larger buffer size you'll need to reduce number of IR heads (see above)
#define SML_MSG_BUFFER 400

// schedule serial output of current SML readings every given number of seconds
#define SML_PRINT_INTERVAL_SECS 5

// don't publish readings if older than given number of seconds
#define SML_DATA_EXPIRE_SECS 60

// MQTT settings
#define MQTT_BROKER "192.168.10.66"
#define MQTT_BROKER_PORT 1883
#define MQTT_BASE_TOPIC "smlreader"
#define MQTT_INTERVAL_SECS 20
//#define MQTT_USERNAME "admin"
//#define MQTT_PASSWORD "xxxxxx"
//#define MQTT_TLS

// time server
#define NTP_ADDRESS "de.pool.ntp.org"

// onboard LED on LolinD32 (flashes on MQTT messages)
#define LED_PIN 5

#define DEBUG_SML
//#define DEBUG_TESTDATA
//#define DEBUG_MEMORY

#endif
