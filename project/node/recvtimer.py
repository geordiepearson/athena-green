#!/usr/bin/env python3

# import socket
from pwn import *
import time
port = 19021

# s = socket.socket()

r = remote('localhost', 19021)

while True:
	d = r.recvuntil(b'\n')
	if b'rssi' in d:
		print(time.time(), d.decode().strip())
