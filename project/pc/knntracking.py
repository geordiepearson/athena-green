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
import pickle
import csv
import pandas as pd
import datetime

client = mqtt.Client()
#knn
knn = pickle.load(open('model_knn.pickle', 'rb'))
NUM_NODE_TRACKED = 12

knn_zone_coords = {
        1: (5, 8.4),
        2: (9, 8.5),
        3: (12, 9),
        4: (18, 8.5),
        5: (23, 8.7),
        6: (26, 8.6),
        7: (29, 8.7),
        8: (35, 9.6)
} 

beacon_coords = { "A" : (4, 8.5),
                  "E" : (10.5, 8.5),
                  "F" : (14.8, 10.5),
                  "G" : (22, 7.6),
                  "P" : (27, 10.5),
                  "Z" : (33.2, 12),
                  # "B" : (0,0),
                  # "C" : (0,0),
                  # "D" : (0,0),
                  
                  # "H" : (0,0),
                  # "I" : (0,0),
                  # "J" : (0,0),
                  # "K" : (0,0),
                  # "L" : (0,0),
                  "static1" : (7, 8.5),
                  "static2" : (19.7, 8.3),
                  "static3" : (26, 9.3),
                  "static4" : (31, 11),
                  "base"    : (13.5, 7.5)
          }

mobile_loc_1 = None
mobile_coords_1 = [13.5, 7.5]
mobile_coords_1_knn = [20, 8]
mobile_loc_2 = None
mobile_coords_2 = [13.5, 7.5]
mobile_coords_2_knn = [10, 7]
mobile_loc_1_knn = None
mobile_loc_2_knn = None
ax = None
family = [[0], [0]]
same = 0

""" Function that converts a RSSI value to a distance based on -59 at 1m.
"""
def dist_to_rssi(rssi, mp=-59, N=4):
   return 10 ** ((mp - rssi) / (10 * N))

def init_graph():
    global ax
    plt.rcParams["figure.figsize"] = [12.7, 12.7]
    plt.rcParams["figure.autolayout"] = True
    im = plt.imread("GP.PNG")

    fig, ax = plt.subplots()
    im = ax.imshow(im, extent = [0, 42, 0, 21.5])
    
    for key in beacon_coords:
        if (len(key) > 1):
            if (key == "base"):
                plt.plot(beacon_coords[key][0], beacon_coords[key][1], marker="^", markersize=10,
                     markeredgecolor="blue", markerfacecolor="blue")
            else:
                plt.plot(beacon_coords[key][0], beacon_coords[key][1], marker="*", markersize=10,
                     markeredgecolor="blue", markerfacecolor="blue")
        else:
            plt.plot(beacon_coords[key][0], beacon_coords[key][1], marker="x", markersize=10,
                     markeredgecolor="blue", markerfacecolor="blue")
    
    mobile_loc_1, = plt.plot(13.5, 7.5, marker="o", markersize=20, markeredgecolor="blue",
             markerfacecolor="red")
    mobile_loc_2, = plt.plot(13.5, 7.5, marker="o", markersize=20, markeredgecolor="blue",
             markerfacecolor="green")
    plt.pause(0.01)
    return (mobile_loc_1, mobile_loc_2)

def init_family():
    global family
    global same
    with open('family.csv') as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        i = 0
        for row in csv_reader:
            family[i] = row
            i += 1
        for fam in family:
            if len(fam) > 1:
                same = 1

def draw_points(coordinates, mobile_id, knn=False):
    global ax
    global mobile_loc_1,mobile_loc_1_knn 
    global mobile_loc_2,mobile_loc_2_knn

    if coordinates[0] > 50 or coordinates[1] > 35 or coordinates[0] < 0 or coordinates[1] < 0:
        return

    if coordinates[1] < 6:
        coordinates = [coordinates[0], coordinates[1] + 6] 

    if mobile_id == 1:
        color = "red"
        if knn and mobile_loc_1_knn != None:
            try:
                ax.lines.remove(mobile_loc_1_knn)
                plt.pause(0.01)

            except ValueError as e:
                print(e)
                pass
            
        elif not knn and mobile_loc_1 != None:
            ax.lines.remove(mobile_loc_1)    
        
    else:
        color = "green"
        if knn and mobile_loc_2_knn != None:
            try:
                ax.lines.remove(mobile_loc_2_knn)
                plt.pause(0.01)

            except ValueError as e:
                print(e)

        elif not knn and mobile_loc_2 != None:
            ax.lines.remove(mobile_loc_2)

    if knn:
        # unpacking a tuple
        point, = plt.plot(coordinates[0], coordinates[1], marker="o", markersize=40, markeredgecolor=color,
                       markerfacecolor=color, alpha=0.6) # marker face white?
    else:
        point, = plt.plot(coordinates[0], coordinates[1], marker="o", markersize=20, markeredgecolor="blue",
                       markerfacecolor=color)
    
    plt.pause(0.01)
    return point

def compute_multilat(rssi_ids, rssi_values):

    distances = []
    positions = []

    for value in rssi_values:
        distances.append(dist_to_rssi(value))
    #print(distances)
    
    for bt_id in rssi_ids:
        if bt_id in beacon_coords:
            positions.append(beacon_coords[bt_id])
    #print(positions)

    if len(positions) < 3:
        return (-1, -1);

    A_x = np.array([2 * (positions[2][0] - positions[0][0]), 2 * (positions[2][0] - positions[1][0])])
    A_y = np.array([2 * (positions[2][1] - positions[0][1]), 2 * (positions[2][1] - positions[1][1])])
    A = np.vstack([A_x, A_y]).T
    
    b = np.array([distances[0]**2 - distances[2]**2 - positions[0][0]**2 - positions[0][1]**2 + positions[2][0]**2 + positions[2][1]**2, 
                  distances[1]**2 - distances[2]**2 - positions[1][0]**2 - positions[1][1]**2 + positions[2][0]**2 + positions[2][1]**2, 
                ])
    #print(np.linalg.lstsq(A, b, rcond=None))
    return np.linalg.lstsq(A, b, rcond=None)
    
def step_to_coordinate(coordinate, step, direction):
    final_coords = coordinate

    if direction == 0:
        final_coords = (coordinate[0], coordinate[1] + (1.2 * step))
    elif direction == 1:
        final_coords = (coordinate[0] + (1.2 * step), coordinate[1])
    elif direction == 2:
        final_coords = (coordinate[0], coordinate[1] - (1.2 * step))
    elif direction == 3:
        final_coords = (coordinate[0] - (1.2 * step), coordinate[1])

    return final_coords

def compute_knn(rssi_ids, rssi_values):
    # fitting to the training data format, converting ids to int
    rssi_id_x = [ord(x) for x in rssi_ids]
    Xdata = {'b1':rssi_id_x[0],'b2':rssi_id_x[1],'b3':rssi_id_x[2],'b1r':rssi_values[0], 'b2r':rssi_values[1], 'b3r':rssi_values[2]}
    
    # for i in range(0,3):
    #     Xdata[i] = [rssi_id_x[0],rssi_id_x[1],rssi_id_x[2],rssi_values[0],rssi_values[1],rssi_values[2]]
    X = pd.DataFrame.from_dict([Xdata])#,index=[0])
    # print(X)
    pred = knn.predict(X)
    return pred


def on_message(client, userdata, message):
    global mobile_loc_1,mobile_loc_1_knn
    global mobile_loc_2,mobile_loc_2_knn
    global mobile_coords_1,mobile_coords_1_knn
    global mobile_coords_2,mobile_coords_2_knn
    global family
    global same

    line = message.payload.decode().strip()
    d = json.loads(line, strict=False)
   
    new_coords = (0, 0)
    rssi_ids = []
    rssi_values = []
    for i in range(1,4):
        rssi_ids.append(d["b" + str(i)])
        rssi_values.append(float(d["b" + str(i) + "r"]))
    steps = int(d["speed"])
    direction = int(d["direction"])
    #print(steps)
    #print(direction)

    multilat = compute_multilat(rssi_ids, rssi_values)
    knn_res = compute_knn(rssi_ids, rssi_values)[0]
    # print('knn res:',knn_res)
    try:
        new_coords = (multilat[0][0], multilat[3][1])
    except TypeError as e:
        new_coords = (-1,-1)

    # knn updates regardless of accel
    if d["mobile_id"] == 1:
        if mobile_coords_1_knn != knn_zone_coords[knn_res]:
            mobile_loc_1_knn = draw_points(knn_zone_coords[knn_res], 1, knn=True)
            mobile_coords_1_knn = knn_zone_coords[knn_res] 
    else:
        # only draw if not the same
        if mobile_coords_2_knn != knn_zone_coords[knn_res]:
            mobile_loc_2_knn = draw_points(knn_zone_coords[knn_res], 2, knn=True)
            mobile_coords_2_knn = knn_zone_coords[knn_res] 
    # print('knn zone (m%d):'% d["mobile_id"],knn_zone_coords[knn_res])

    if new_coords[0] != -1 and new_coords[1] != -1:
        if int(d["mobile_id"]) == 1:
            dif = math.sqrt(((new_coords[0] - mobile_coords_1[0]) ** 2 +
                (new_coords[1] - mobile_coords_1[1]) ** 2))
            """if dif > 2:
                final_coords = step_to_coordinate(mobile_coords_1, steps, direction)
            else:
                final_coords = step_to_coordinate(new_coords, steps, direction)"""
            final_coords = step_to_coordinate(new_coords, steps, direction)
            mobile_coords_1 = final_coords
        else:
            dif = math.sqrt(((new_coords[0] - mobile_coords_2[0]) ** 2 +
                (new_coords[1] - mobile_coords_2[1]) ** 2))
            """if dif > 2:
                final_coords = step_to_coordinate(mobile_coords_2, steps, direction)
            else:
                final_coords = step_to_coordinate(new_coords, steps, direction)"""
            final_coords = step_to_coordinate(new_coords, steps, direction)
            mobile_coords_2  = final_coords
        
        if int(d["mobile_id"]) == 1:
            mobile_loc_1 = draw_points(final_coords, 1)
        else:
            mobile_loc_2 = draw_points(final_coords, 2)
            


    
    if same == 0:
        if mobile_coords_1_knn == mobile_coords_2_knn:
            print("COVID BAD (knn)", datetime.datetime.now())
        if math.sqrt((mobile_coords_1[0] - mobile_coords_2[0])** 2 +
                     (mobile_coords_1[1] - mobile_coords_2[1])** 2) < 2:
            print("COVID BAD (mlat)", datetime.datetime.now())

def on_log(client, userdata, level, buf):
    print("log: " + buf)

def on_disconnect(client, userdata, rc):
    if rc != 0:
        print("Unexpected disconnection.")

def on_connect(client, userdata, flags, rc):
    client.subscribe('base')
    if rc == 0:
        print("connected OK")
    else:
        print("Bad connection returnd code = ", rc)

mobile_loc_1, mobile_loc_2 = init_graph()
init_family()
client.on_connect = on_connect
client.on_disconnect = on_disconnect
client.on_message = on_message

client.connect("127.0.0.1", 1883)
time.sleep(1)
client.subscribe('base')
client.loop_forever()


"""while(1):
    mobile_loc_1 = draw_points((0,0), 1)
    mobile_loc_2 = draw_points((10, 0), 2)
    time.sleep(1)
    
    mobile_loc_1 = draw_points((1, 1), 1)
    mobile_loc_2 = draw_points((10, 5), 2)
    time.sleep(1)"""
