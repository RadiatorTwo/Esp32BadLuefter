// config.cpp
#include "config.h"

// WiFi-Konfiguration
const char* HOSTNAME = "HOSTNAME";
const char* WIFI_SSID = "WIFI";
const char* WIFI_PASSWORD = "PASSWORD";

// Netzwerk-Konfiguration
const IPAddress LOCAL_IP(192, 168, 1, 10);
const IPAddress GATEWAY(192, 168, 1, 1);
const IPAddress SUBNET(255, 255, 255, 0);
const IPAddress PRIMARY_DNS(192, 168, 1, 1);
const IPAddress SECONDARY_DNS(192, 168, 1, 2);
