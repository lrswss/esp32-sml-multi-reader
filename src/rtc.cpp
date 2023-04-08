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

#include "config.h"
#include "rtc.h"

// setup the ntp udp client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, 0, 300);

// TimeZone settings details https://github.com/JChristensen/Timezone
TimeChangeRule CEST = { "CEST", Last, Sun, Mar, 2, 120 };
TimeChangeRule CET = { "CET", Last, Sun, Oct, 3, 60 };
Timezone TZ(CEST, CET);  // Frankfurt, Paris  // keep "TZ"


// return abbreviation for current timezone
static char* getTimeZone() {
    static char tz[6];
    TimeChangeRule *tcr; 
    time_t now;

    time(&now);
    TZ.toLocal(now, &tcr);
    strncpy(tz, tcr->abbrev, 5);
    return tz;
}


// returns local time (seconds since epoch)
static time_t getLocalTime() {
    time_t time_utc;
    time(&time_utc);
    return TZ.toLocal(time_utc);
}


// returns current system time (UTC) as string
static char* getSystemTime() {
    static char str[32];
    time_t now;

    now = getLocalTime();
    strftime(str, sizeof(str), "%Y/%m/%d %H:%M:%S (", localtime(&now));
    strcat(str, getTimeZone());  // add timezone abbreviation
    strcat(str, ")");
    return str;
}


// returns system's total runtime 
char* getRuntime() {
    static uint32_t lastCall = 0;
    static uint32_t runtimeTenthSecs = 0;
    static char str[16];

    runtimeTenthSecs += int((millis() - lastCall)/100);
    uint32_t runtimeSecs = runtimeTenthSecs/10;
    lastCall = millis();

    memset(str, 0, sizeof(str));
    uint16_t days = runtimeSecs / 86400 ;
    uint8_t hours = (runtimeSecs % 86400) / 3600;
    uint8_t minutes = ((runtimeSecs % 86400) % 3600) / 60;
    sprintf(str, "%dd %dh %dm", days, hours, minutes);
    return str;
}


// start ntp client and update RTC
bool startNTPSync() {
    struct timeval tv;

    Serial.print(millis());
    timeClient.begin();
    timeClient.forceUpdate(); // takes a while
    if (timeClient.getEpochTime() > 1000) {
        // set ESP32 internal RTC
        tv.tv_sec = timeClient.getEpochTime(); // epoch
        tv.tv_usec = 0;
        settimeofday(&tv, NULL); // TZ UTC
        Serial.printf(": Local time: %s\n", getSystemTime());
        return true;
    } else {
        Serial.println(F(": Syncing RTC with NTP-Server failed!"));
        return false;
    }
}


void stopNTPSync() {
    timeClient.end();
}