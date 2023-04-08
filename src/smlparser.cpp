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
#include "rtc.h"
#include "utils.h"
#include "config.h"

sml_states_t currentState;

OBISHandler OBISHandlers[] = {
    { { 0x81, 0x81, 0xc7, 0x82, 0x03, 0xff }, &Manufacturer },         /* 129-129:199.130.3*255 */
    { { 0x01, 0x00, 0x60, 0x32, 0x01, 0x01 }, &Manufacturer },         /* 1-0:96.50.1*1 */
    { { 0x01, 0x00, 0x01, 0x08, 0x00, 0xff }, &EnergyFromGridTotal },  /* 1-0:1.8.0*255 (Total Power From Grid T1+T2) */
    { { 0x01, 0x00, 0x02, 0x08, 0x00, 0xff }, &EnergyToGridTotal },    /* 1-0:2.8.0*255 (Total Power To Grid T1+T2) */    
    { { 0x01, 0x00, 0x10, 0x07, 0x00, 0xff }, &PowerFromGridTotal },   /* 1-0:16.7.0*255 (Active Power from Grid) */
    { { 0x01, 0x00, 0x0F, 0x07, 0x00, 0xff }, &PowerToGridTotal },     /* 1-0:15.7.0*255 (Active Power to Grid) */
    { { 0x01, 0x00, 0x24, 0x07, 0x00, 0xff }, &PowerFromGridL1 },      /* 1-0:36.7.0*255 (Active Power L1) */
    { { 0x01, 0x00, 0x38, 0x07, 0x00, 0xff }, &PowerFromGridL2 },      /* 1-0:56.7.0*255 (Active Power L2) */
    { { 0x01, 0x00, 0x4C, 0x07, 0x00, 0xff }, &PowerFromGridL3 },      /* 1-0:76.7.0*255 (Active Power L3) */
    { { 0x01, 0x00, 0x15, 0x07, 0x00, 0xff }, &PowerFromGridL1 },      /* 1-0:21.7.0*255 (Active Power L1) */
    { { 0x01, 0x00, 0x29, 0x07, 0x00, 0xff }, &PowerFromGridL2 },      /* 1-0:41.7.0*255 (Active Power L2) */
    { { 0x01, 0x00, 0x3D, 0x07, 0x00, 0xff }, &PowerFromGridL3 },      /* 1-0:61.7.0*255 (Active Power L3) */
    { { 0x01, 0x00, 0x60, 0x01, 0x00, 0xff }, &Serialnumber },         /* 1-0:96.1.0*255 */
    { { 0x01, 0x00, 0x00, 0x00, 0x09, 0xff }, &Serialnumber },         /* 1-0:0.0.9*255 */
    { { 0 }, 0}
};


void resetSMLReadings(SMLDeviceReadings *data) {
    memset(data->manufacturer, 0, sizeof(data->manufacturer));
    memset(data->serialnumber, 0, sizeof(data->serialnumber));
    memset(data->fullMessage, 0, sizeof(data->fullMessage));
    data->energyFromGridTotal = LONG_MIN;
    data->energyToGridTotal = LONG_MIN;
    data->powerFromGridTotal = LONG_MIN;
    data->powerToGridTotal = LONG_MIN;
    data->powerFromGridL1 = LONG_MIN;
    data->powerFromGridL2 = LONG_MIN;
    data->powerFromGridL3 = LONG_MIN;
    data->timestamp = 0;
    data->state = SML_VERSION;
}


bool readSMLByte(byte c, SMLDeviceReadings *data) {
    static uint32_t lastErrMsgMillis = 0;
    static uint16_t frameCounter = 0;
    time_t time_utc;
    uint8_t iHandler = 0;
    char* msg;
    
    currentState = smlState(c);
    if (frameCounter != 0 && currentState == SML_START) {
        resetSMLReadings(data);
        frameCounter = 0;
    }

    // copy of full message used to parse serial number and manufacturer
    if (frameCounter < sizeof(data->fullMessage)) {
        data->fullMessage[frameCounter++] = c;
        data->msgSize = frameCounter;
    }

    if (currentState == SML_LISTEND) {
        for (iHandler = 0; OBISHandlers[iHandler].Handler != 0 &&
                !(smlOBISCheck(OBISHandlers[iHandler].OBIS)); iHandler++);
        if (OBISHandlers[iHandler].Handler != 0) {
            OBISHandlers[iHandler].Handler(data, OBISHandlers[iHandler].OBIS);
        }
    }

    if (frameCounter >= sizeof(data->fullMessage)) {
        xSemaphoreTake(SerialLock, portMAX_DELAY);
        Serial.printf("%ld: SML buffer exceeded (%d bytes)\n", millis(), frameCounter);
        xSemaphoreGive(SerialLock); 
        time(&time_utc);
        data->timestamp = time_utc;
        data->state = SML_END;
        frameCounter = 0;
        return true;
    }

    if (currentState == SML_UNEXPECTED) {
        if (millis() - lastErrMsgMillis > 1000) {
            xSemaphoreTake(SerialLock, portMAX_DELAY);
            Serial.printf("%ld: Received unexpected byte\n", millis());
            xSemaphoreGive(SerialLock); 
            lastErrMsgMillis = millis();
        }
    }

    if (frameCounter != 0 && currentState == SML_CHECKSUM_ERROR) {
        xSemaphoreTake(SerialLock, portMAX_DELAY);
        Serial.printf("%ld: Received SML message with invalid checksum on pin %d (%d bytes)\n", 
            millis(), data->pin, frameCounter);
        xSemaphoreGive(SerialLock); 
        time(&time_utc);
        data->timestamp = time_utc;
        data->state = SML_CHECKSUM_ERROR;
        frameCounter = 0;
        return true;

    } else if (frameCounter != 0 && currentState == SML_FINAL) {
        xSemaphoreTake(SerialLock, portMAX_DELAY);
        Serial.printf("%ld: Received and parsed SML message on pin %d (%d bytes)\n", 
            millis(), data->pin, frameCounter);
        xSemaphoreGive(SerialLock); 
        time(&time_utc);
        data->timestamp = time_utc;
        data->state = SML_FINAL;
        frameCounter = 0;
        return true;
    }

    return false;
}


void printSMLReadings(const SMLDeviceReadings &data) {
    char buf[16], timeStr[24];
#ifdef DEBUG_SML
    uint16_t i = 0, j = 2;
    static char smlmsg[1024];
#endif
    time_t time_utc;
    struct tm tm;

    time(&time_utc);
    if (data.state == SML_CHECKSUM_ERROR) {
        Serial.println(F("  Invalid data (checksum error)"));
    } else if (time_utc - data.timestamp < SML_DATA_EXPIRE_SECS) {
        localtime_r(&time_utc, &tm);
        sprintf(timeStr, "%4d-%.2d-%.2dT%.2d:%.2d:%.2d",
            tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec);
        Serial.printf("  Timestamp: %s\n", timeStr);
        Serial.printf("  Manufacturer: %s\n", data.manufacturer);
        Serial.printf("  Serialnumber: %s\n", data.serialnumber);
        if (data.energyFromGridTotal > LONG_MIN) {
            dtostrf(data.energyFromGridTotal/1000, 12, 3, buf);
            Serial.printf("  Total Consumption: %s kWh\n", removeSpaces(buf));
        }
        if (data.energyToGridTotal > LONG_MIN) {
            dtostrf(data.energyToGridTotal/1000, 12, 3, buf);
            Serial.printf("  Total Feed to Grid: %s kWh\n", removeSpaces(buf));
        }
        if (data.powerFromGridTotal > LONG_MIN)
            Serial.printf("  Total Active Power: %d W\n", int(data.powerFromGridTotal));        
        if (data.powerFromGridL1 > LONG_MIN)
            Serial.printf("  Active Power L1: %d W\n", int(data.powerFromGridL1));
        if (data.powerFromGridL2 > LONG_MIN)
            Serial.printf("  Active Power L2: %d W\n", int(data.powerFromGridL2));
        if (data.powerFromGridL3 > LONG_MIN)
            Serial.printf("  Active Power L3: %d W\n", int(data.powerFromGridL3));
        if (data.powerToGridTotal > LONG_MIN)
            Serial.printf("  Total Active Power to Grid: %d W\n", int(data.powerToGridTotal));
#ifdef DEBUG_SML
        memset(smlmsg, 0, sizeof(smlmsg));
        arr2str(data.fullMessage, data.msgSize, smlmsg);
        Serial.print(F("  Raw SML message:\n    1B"));
        while (i < strlen(smlmsg)) {
            Serial.print(smlmsg[i++]);
            if (j++ > 64) {
                Serial.print("\n");
                if (i < strlen(smlmsg))
                    Serial.print("    ");
                j = 0;
            }
        }
        if (j != 0)
            Serial.println();
#endif
    } else if (strlen((char*)data.manufacturer) > 0) {
        Serial.println(F("  No recent data"));
    } else {
        Serial.println(F("  No data"));
    }
}