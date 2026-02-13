# BLE-IoT-SensorNetwork

A Bluetooth Low Energy (BLE) sensor network using ESP32 devices and a Raspberry Pi gateway to collect sensor data and publish it to the cloud via MQTT.

## Overview

This project creates a wireless sensor network in an office environment using BLE-enabled ESP32 microcontrollers as sensor nodes and a Raspberry Pi as a central gateway. The gateway converts BLE notifications into MQTT messages and forwards them to a cloud broker for storage, analysis, and visualization.

## Architecture

```
[ESP32 Sensors]  <--BLE-->  [Raspberry Pi]  <--WiFi/Ethernet-->  [MQTT Broker]  <-->  [Web Dashboard]
 - Temperature              (Central Gateway)                       (Cloud)              (Visualization)
 - Humidity
 - Motion
 - Light
 - etc.
```

### Key Components

- **ESP32 Sensor Nodes**: Battery-powered BLE peripherals that collect environmental data
- **Raspberry Pi Gateway**: BLE central device that scans for sensors and bridges to MQTT
- **MQTT Broker**: Cloud-based message broker for data distribution
- **Web Dashboard**: Visualization of sensor data over time
