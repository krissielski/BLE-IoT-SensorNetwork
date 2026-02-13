# Central Gateway (Raspberry Pi)

BLE-to-MQTT bridge that connects BLE sensor nodes to the cloud.

## Role

The Raspberry Pi acts as a **BLE Central** device that:
- Scans for and connects to BLE sensor nodes
- Subscribes to sensor notifications
- Converts BLE data to MQTT messages
- Publishes sensor data to cloud broker

