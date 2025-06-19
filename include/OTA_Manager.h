#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

//* ************************************************************************
//* ************************ OTA FUNCTION DECLARATIONS *******************
//* ************************************************************************

void initWiFi();
void initOTA();
void handleOTA();
void displayIP();

#endif 