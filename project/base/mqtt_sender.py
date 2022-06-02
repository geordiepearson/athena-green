#!/usr/bin/env python3
import paho.mqtt.client as mqtt
import time
import argparse
import serial
import json

port = serial.Serial()
port.port = "/dev/ttyACM0"


shellprompt=b"\r\x1b[1;32mSHELLY>"

def publish(client, topic, message):
    client.publish(topic, message)

def on_log(client, userdata, level, buf):
    print ("log: " + buf)

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print ("connected OK")
    else:
        print ("Bad connection returned code =", rc)

def on_disconnect(client, userdata, rc):
    if rc != 0:
        print("Unexpected disconnection.")

def on_message(client, userdata, message):
    print (message.payload)

def main(args):
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_disconnect = on_disconnect
    client.on_message = on_message
        
    if args.verbose:
        client.on_log = on_log
    print ("connecting to broker", args.host)
    client.connect(args.host, args.port)
    time.sleep(2)
    # client.subscribe(args.topic)
    with port as s:
            print("serial connected.")
            while 1:
                msg = s.readline()#(80).split(b'\n')
                # data = s.readline()
                # for msg in data.split(b'\n'):
                    # print('msgs:', msgs)
                    # for msg in msgs:
                msg = msg.replace(shellprompt,b'')
                # print(msg)
                if len(msg.split(b"{")) > 1:
                    msg = msg.split(b"{")[1].split(b"}")[0] 
                    msg = b"{" + msg.strip(b'\n') + b"}"
                    msg = msg.decode('utf-8','ignore')
                    print('publishing:',msg)
                    publish(client, args.topic, msg)
    client.loop_forever(timeout=100000)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-v', action='store_true', dest='verbose', required=False)
    parser.add_argument('-H', action='store', dest='host', required=False, default="localhost")
    parser.add_argument('-p', action='store', dest='port', type=int, required=False, default="1883")
    parser.add_argument('-t', action='store', dest='topic', required=True)
    args = parser.parse_args()

    main(args)

    
