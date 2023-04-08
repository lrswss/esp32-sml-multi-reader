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

#ifndef _SMLREADER_H
#define _SMLREADER_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <list>
#include "smlreader.h"
#include "smlparser.h"

#define SML_READER_INTERVAL_MS 5000

class SMLReader {
    public:
        SMLReader();
        SMLReader(const uint8_t);
        bool begin(const uint8_t);
        void read();
        void startReader();
        void printReadings();
        void startPrinter();
        SMLDeviceReadings getReadings();
    private:
        void readingTask();
        void printerTask();
        static void printerTaskWrapper(void*);
        static void readingTaskWrapper(void*);
        std::unique_ptr<SoftwareSerial> ss;
        SMLDeviceReadings readings;
};

extern std::list<SMLReader *> *smlreaderList;

#endif
