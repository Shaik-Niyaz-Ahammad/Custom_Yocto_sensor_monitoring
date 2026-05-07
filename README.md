                        Embedded Monitoring System with OTA Updates (Yocto + BBB)

📌 Overview
This project is an embedded Linux-based monitoring system built on the BeagleBone Black using Yocto.  
It collects environmental data from a BME280 sensor and implements a complete OTA (Over-The-Air) update mechanism with version control, checksum verification, and automatic updates via systemd timers.

🚀 Key Features

🌡️ Real-time sensor monitoring (Temperature, Pressure, Humidity)
📝 Configurable via `/etc/sensor_app.conf`
📂 Persistent logging to `/var/log/sensor.log`
⚙️ Managed using `systemd` service
🔄 OTA update system:
        Version-based update mechanism
        Binary + config update
        SHA256 checksum verification
        Rollback on failure
⏱️ Automatic updates using `systemd timer`
🧱 Fully integrated into Yocto image (no manual setup required)

Sensor Service
    Runs continuously via `systemd`
    Reads BME280 sensor data
    Logs output every configurable interval

OTA Update Flow
    Timer triggers `update_sensor.service`
    Script downloads `version.txt`
    Compares with local version
    If newer version:
        Downloads binary + config
        Verifies checksum
        Stops service
        Replaces files
        Restarts service
    If failure → rollback to previous version

🧠 Key Engineering Decisions
Used systemd timer instead of cron → better integration with Yocto
Added checksum verification → prevents corrupted updates
Used separate version file → avoids blocking service during version check
Implemented rollback → improves system reliability


🛠️ Tech Stack
Embedded C
Yocto Project
Linux (systemd, journald)
Networking (HTTP)
Shell scripting
BeagleBone Black

📈 Future Improvements
Secure OTA (HTTPS)
Remote cloud-based OTA server
MQTT-based telemetry
Web dashboard

