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

#ifndef _UTILS_H
#define _UTILS_H

#include <Arduino.h>
#include "esp_task_wdt.h"
#include "esp_sleep.h"
#include "esp_system.h"
#include "esp_ota_ops.h"

#define WATCHDOG_TIMEOUT_SEC 90

extern SemaphoreHandle_t SerialLock;

void arr2str(const char *arr, int len, char *buf);
void blinkLED(uint8_t repeat, uint16_t pause);
char* removeSpaces(char *str);
void switchLED(bool state);
void startWatchdog();
void stopWatchdog();
String systemID();
void printFreeStackWatermark(const char *taskName);
void debugTask(void* parameter);

#endif