import paho.mqtt.client as mqtt

import config

#       root/site/area/type/deviceID/topic
TOPIC = "iot/+/+/test/+/data"


def on_message(client: mqtt.Client, userdata, msg: mqtt.MQTTMessage):
    print(msg.topic)
    print(msg.payload.decode("utf-8", errors="replace"))


def main():
    client = mqtt.Client()
    client.username_pw_set(config.USERNAME, config.PASSWORD)
    client.tls_set()

    client.on_message = on_message
    client.connect(config.BROKER_HOST, config.BROKER_PORT, keepalive=60)
    client.subscribe(TOPIC, qos=0)

    try:
        client.loop_forever()
    except KeyboardInterrupt:
        pass
    finally:
        client.disconnect()


if __name__ == "__main__":
    main()
