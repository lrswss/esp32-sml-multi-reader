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

#ifndef _WLAN_H
#define _WLAN_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>

extern WiFiManager wm;

#define WIFI_AP_SSID "SML-Multi-Reader"
#define WIFI_MIN_RSSI 30
#define WIFI_CONFIG_TIMEOUT_SECS 300
#define WIFI_CONNECT_TIMEOUT_SECS 15
#define WIFI_CHECK_SECS 30
#define WIFI_RETRY_SECS 10

void startWifi();
void wifiReconnect();

#endif