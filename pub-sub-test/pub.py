import json
import time
from datetime import datetime, timezone

import paho.mqtt.client as mqtt

import config

#       root/site/area/type/deviceID/topic
TOPIC = "iot/site/area/test/dev999/data"


PUBLISH_PERIOD  = 10  #seconds


def build_payload():
    payload = {
        "type": "test",
        "message": "hello",
        "timestamp": datetime.now(timezone.utc).isoformat(),
    }
    return json.dumps(payload)


def main():
    client = mqtt.Client()

    client.username_pw_set(config.USERNAME, config.PASSWORD)

    client.tls_set()

    client.connect(config.BROKER_HOST, config.BROKER_PORT, keepalive=60)
    client.loop_start()


    while True:
        try:
            payload = build_payload()
            result = client.publish(TOPIC, payload, qos=0, retain=False)
            result.wait_for_publish()
            print("Publish")
            time.sleep(PUBLISH_PERIOD)
        except KeyboardInterrupt:
            break

    client.loop_stop()
    client.disconnect()


if __name__ == "__main__":
    main()
