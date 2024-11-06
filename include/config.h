// config.h
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// WiFi-Konfiguration
extern const char* HOSTNAME;
extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;

// Netzwerk-Konfiguration
extern const IPAddress LOCAL_IP;
extern const IPAddress GATEWAY;
extern const IPAddress SUBNET;
extern const IPAddress PRIMARY_DNS;    // optional
extern const IPAddress SECONDARY_DNS;  // optional

#endif // CONFIG_H
