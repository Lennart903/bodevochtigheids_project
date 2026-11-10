# IoT Bodemvochtigheid Monitoringssysteem (ESP32-C3 & Supabase)

Een energiezuinig, op IoT gebaseerd monitoringssysteem voor bodemvochtigheid, gebouwd met twee **ESP32-C3** modules. Het systeem bevat een **Sensor Node** (op zonne-energie, met deep-sleep) die draadloos via **ESP-NOW** communiceert met een **Hub Node** (altijd aan). Deze Hub stuurt de data via HTTPS door naar een **Supabase** cloud database.

## Probleemstelling: De Wi-Fi Dode Hoek

**Hoe kun je betrouwbaar sensordata verzamelen en verzenden vanuit een afgelegen locatie, zoals een tuin, waar je Wi-Fi-netwerk thuis geen bereik heeft?**

Standaard Wi-Fi verbruikt veel stroom en heeft een beperkt bereik, waardoor het vaak moeite heeft om door buitenmuren te dringen en de tuin te bereiken. Dit project lost dat probleem op door een 'bridge'-architectuur met twee nodes te gebruiken:

1. **De Tuin (Sensor Node):** Gebruikt **ESP-NOW**, een gespecialiseerd, extreem energiezuinig protocol zonder router. Het kan eenvoudig data over grotere afstanden (vaak verder dan standaard Wi-Fi) direct naar een andere ESP32 sturen, zonder dat er een actieve internetverbinding in de tuin nodig is.

2. **Het Huis (Hub Node):** Wordt geplaatst aan de rand van het huis (of binnen) waar nog wel een stabiel Wi-Fi signaal is. Deze node ontvangt het ESP-NOW signaal uit de tuin, fungeert als brug naar het internet en stuurt de data direct naar de cloud.

## Systeemarchitectuur

```text
[ Sensor Node (Deep Sleep) ] 
       │
       │ (ESP-NOW / 2.4GHz Wi-Fi Kanaal 11 - Zonder router)
       ▼
[ Hub Node (Altijd aan) ] 
       │
       │ (HTTPS / REST API - Standaard Wi-Fi)
       ▼
[ Supabase Database ] ──> [ GitHub Pages Web Dashboard (Chart.js) ]
```

## Hardware Componenten

* **Microcontrollers:** 2x ESP32-C3 Mini modules
* **Sensor:** Capacitieve bodemvochtigheidssensor (analoge uitgang aangesloten op het ADC1 kanaal)
* **Voeding:** Zonnepaneel + Batterijbeheer voor de Sensor Node / 5V USB voor de Hub Node
* **Statusindicatie:** Ingebouwde / Externe LED's voor verbindings- en transmissiestatussen

## Repository Structuur

```text
├── hub_station/              # Hub Node Firmware (ESP-IDF)
│   ├── main/
│   │   ├── main.c            # Initialiseert Wi-Fi en ESP-NOW
│   │   ├── connection_setup.c# Beheert Wi-Fi APSTA modus, HTTP uploads en ESP-NOW callbacks
│   │   └── connection_setup.h
│   └── CMakeLists.txt
│
├── sensor_station/           # Sensor Node Firmware (ESP-IDF)
│   ├── main/
│   │   ├── main.c            # Leest ADC uit, verzendt datapakket, wacht op ACK, start deep-sleep
│   │   ├── esp_now_setup.c   # ESP-NOW configuratie en peer-beheer
│   │   └── esp_now_setup.h
│   └── CMakeLists.txt
│
└── web_dashboard/            # Frontend Web UI
    └── index.html            # Real-time Chart.js interface gekoppeld aan de Supabase REST API
```

## Configuratie & Installatie

### 1. Hub Node Configuratie (`connection_setup.c`)
Pas je Wi-Fi-gegevens en Supabase projectsleutels aan:
```c
#define WIFI_SSID         "JOUW_WIFI_SSID"
#define WIFI_PASSWORD     "JOUW_WIFI_WACHTWOORD"
#define SUPABASE_URL      "https://<jouw-project>.supabase.co/rest/v1/sensor_readings"
#define SUPABASE_KEY      "JOUW_SUPABASE_PUBLISHABLE_KEY"
```

### 2. Sensor Node Configuratie (`esp_now_setup.c`)
Zorg dat het Wi-Fi-kanaal overeenkomt met je router en stel het **AP MAC-adres** van de Hub in (te vinden in de opstartlogs van de Hub):
```c
#define ESP_NOW_CHANNEL 11
static const uint8_t HUB_MAC_ADDR[] = {0xB8, 0x1F, 0x3F, 0x0F, 0x81, 0xF1};
#define READ_INTERVAL_SEC 60
```

## Supabase Database Schema

Maak een tabel genaamd `sensor_readings` aan in je Supabase project met de volgende kolommen:

* `id` (int8, Primary Key, Standaardwaarde: auto-increment)
* `created_at` (timestamptz, Standaardwaarde: `now()`)
* `humidity` (float4)

## Flashen en Uitvoeren

1. Open je terminal in de betreffende projectmap en gebruik de ESP-IDF tools.
2. Build en flash de **Hub Node**:
   ```bash
   idf.py -p COM<poort> flash monitor
   ```
3. Build en flash de **Sensor Node**:
   ```bash
   idf.py -p COM<poort> flash monitor
   ```
4. Host het `index.html` bestand via **GitHub Pages** om de actuele bodemvochtigheidsgrafiek in de loop der tijd te bekijken.
