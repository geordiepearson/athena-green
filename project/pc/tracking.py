#!/usr/bin/env python3

""" Script to peform realtime data processing and data display.
"""
import json
import sys
import math
import numpy as np
import matplotlib.pyplot as plt
import time
import paho.mqtt.client as mqtt

client = mqtt.Client()

NUM_NODE_TRACKED = 12

beacons = { 'F5:75:FE:85:34:67':"A",
            'E5:73:87:06:1E:86':"B",
            'CA:99:9E:FD:98:B1':'C',
            'CB:1B:89:82:FF:FE':'D',
            'F9:BD:57:FF:25:04':'E',
            'C1:13:27:E9:B7:7C':'F',
            'F1:04:48:06:39:A0':'G',
            'CA:0C:E0:DB:CE:60':'H',
            'D4:7F:D4:7C:20:13':'I',
            'F7:0B:21:F1:C8:E1':'J',
            'FD:E0:8D:FA:3E:4A':'K',
            'EE:32:F7:28:FA:AC':'L'
          }

beacon_coords = { "A" : (0,0),
                  "B" : (0,0),
                  "C" : (0,0),
                  "D" : (0,0),
                  "E" : (0,0),
                  "F" : (0,0),
                  "G" : (0,0),
                  "H" : (0,0),
                  "I" : (0,0),
                  "J" : (0,0),
                  "K" : (0,0),
                  "L" : (0,0),
                  "static1" : (0,0),
                  "static2" : (0,0),
                  "static3" : (0,0),
                  "static4" : (0,0)
          }

""" Function that converts a RSSI value to a distance based on -59 at 1m.
"""
def dist_to_rssi(rssi, mp=-59, N=4):
   return 10 ** ((mp - rssi) / (10 * N))

def init_graph():
    plt.rcParams["figure.figsize"] = [12.7, 12.7]
    plt.rcParams["figure.autolayout"] = True
    im = plt.imread("GP.PNG")

    fig, ax = plt.subplots()
    im = ax.imshow(im, extent = [0, 1000, 0, 500])
    
    
def draw_points(multi_lat, knn):
    point_m, = plt.plot(multi_lat[0], multi_lat[1], marker="o", markersize=30, markeredgecolor="red",
                       markerfacecolor="red")
    point_k, = plt.plot(knn[0], knn[1], marker = "o", markersize=30, markeredgecolor="green",
                       markerfacecolor="green")
    plt.pause(0.01)
    return (point_m, point_k)

init_graph()
while(1):
    multi, knn = draw_points((0,0), (100,100))
    time.sleep(2)
    multi.remove()
    knn.remove()

    multi, knn = draw_points((10, 10), (1000, 500))
    time.sleep(2)
    multi.remove()
    knn.remove()
