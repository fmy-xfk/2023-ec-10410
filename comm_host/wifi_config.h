/*
 * Copyright (c) 2022 HiSilicon (Shanghai) Technologies CO., LIMITED.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef UDP_CONFIG_H
#define UDP_CONFIG_H

#define UDP_DEMO

#ifdef UDP_DEMO
#define UDP_DEDMO_SUPPORT
#define CONFIG_WIFI_STA_MODULE
#elif defined(UDP_AP_DEMO)
#define UDP_DEDMO_SUPPORT
#define CONFIG_WIFI_AP_MODULE
#endif
/**
  @brief enable HW iot cloud
 * HW iot cloud send message to Hi3861 board and Hi861 board publish message to HW iot cloud
 */

// /<CONFIG THE LOG
/* if you need the iot log for the development ,
please enable it, else please comment it
*/
#define CONFIG_LINKLOG_ENABLE   1

// /<CONFIG THE WIFI
/* Please modify the ssid and pwd for the own */
#define AP_SSID  "LT0000" // WIFI SSID
#define AP_PWD   "12348765" // WIFI PWD

#endif