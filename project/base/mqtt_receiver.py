#!/usr/bin/env python3
import paho.mqtt.client as mqtt
import time
import argparse
# import serial
import json,sys

# port = serial.Serial()
# port.port = "/dev/ttyACM0"




def publish(client, topic, message):
    client.publish(topic, message)

def on_log(client, userdata, level, buf):
    print ("log: " + buf)

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print ("connected OK")
        # with port as s:
        #     while 1:
        #         msg = s.read(80)
        #         d = json.loads(msg.decode())
        #         did = list(d.keys()[0])
        #         values = str(d[did])
        #         publish(client, did, values)
    else:
        print ("Bad connection returned code =", rc)

def on_disconnect(client, userdata, rc):
    if rc != 0:
        print("Unexpected disconnection.")

def on_message(client, userdata, message):
    try:
        print (message.payload.decode())
    except Exception as e:
        print(e, file=sys.stderr)

def main(args):
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_disconnect = on_disconnect
    client.on_message = on_message
        
    if args.verbose:
        client.on_log = on_log
    print ("connecting to broker", args.host)
    client.connect(args.host, args.port)
    time.sleep(1)
    client.subscribe(args.topic)
    client.loop_forever()

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-v', action='store_true', dest='verbose', required=False)
    parser.add_argument('-H', action='store', dest='host', required=False, default="localhost")
    parser.add_argument('-p', action='store', dest='port', type=int, required=False, default="1883")
    parser.add_argument('-t', action='store', dest='topic', required=True)
    args = parser.parse_args()

    main(args)

    
