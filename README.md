# 🌱 IoT Soil Moisture Monitoring System (ESP32-C3 & Supabase)

A low-power, IoT-based soil moisture monitoring system built using two **ESP32-C3** modules. The system features a solar-powered, deep-sleeping **Sensor Node** that communicates wirelessly via **ESP-NOW** to an always-on **Hub Node**, which relays data to a **Supabase** cloud database via HTTPS.

---

## System Architecture

```text
[ Sensor Node (Deep Sleep) ] 
       │
       │ (ESP-NOW / 2.4GHz Wi-Fi Channel 11)
       ▼
[ Hub Node (Always On) ] 
       │
       │ (HTTPS / REST API)
       ▼
[ Supabase Database ] ──> [ GitHub Pages Web Dashboard (Chart.js) ]
```

---

## Hardware Components

* **Microcontrollers:** 2x ESP32-C3 Mini modules
* **Sensor:** Capacitive Soil Moisture Sensor (analog output connected to ADC channel)
* **Status Indication:** Onboard / External LEDs for connection and transmission states

---

## Repository Structure

```text
├── hub_station/              # Hub Node Firmware (ESP-IDF)
│   ├── main/
│   │   ├── main.c            # Initializes Wi-Fi and ESP-NOW
│   │   ├── connection_setup.c# Handles Wi-Fi APSTA mode, HTTP uploads, and ESP-NOW callbacks
│   │   └── connection_setup.h
│   └── CMakeLists.txt
│
├── sensor_station/           # Sensor Node Firmware (ESP-IDF)
│   ├── main/
│   │   ├── main.c            # Reads ADC, transmits packet, awaits ACK, enters deep sleep
│   │   ├── esp_now_setup.c   # ESP-NOW configuration and peer handling
│   │   └── esp_now_setup.h
│   └── CMakeLists.txt
│
└── web_dashboard/            # Frontend Web UI
    └── index.html            # Real-time Chart.js interface connected to Supabase REST API
```

---

## Configuration & Setup

### 1. Hub Node Configuration (`connection_setup.c`)
Update your Wi-Fi credentials and Supabase project keys:
```c
#define WIFI_SSID         "YOUR_WIFI_SSID"
#define WIFI_PASSWORD     "YOUR_WIFI_PASSWORD"
#define SUPABASE_URL      "https://<your-project>.supabase.co/rest/v1/sensor_readings"
#define SUPABASE_KEY      "YOUR_SUPABASE_PUBLISHABLE_KEY"
```

### 2. Sensor Node Configuration (`esp_now_setup.c`)
Match the Wi-Fi channel with your router and set the Hub's **AP MAC address** (discovered during Hub startup logs):
```c
#define ESP_NOW_CHANNEL 11
static const uint8_t HUB_MAC_ADDR[] = {0xB8, 0x1F, 0x3F, 0x0F, 0x81, 0xF1};
#define READ_INTERVAL_SEC 60
```

---

## Supabase Database Schema

Create a table named `sensor_readings` in your Supabase project with the following columns:

* `id` (int8, Primary Key, Default: auto-increment)
* `created_at` (timestamptz, Default: `now()`)
* `humidity` (float4)

---

## Flashing and Running

1. Open your terminal in the respective project directory using ESP-IDF environment tools.
2. Build and flash the **Hub Node**:
   ```bash
   idf.py -p COM<port> flash monitor
   ```
