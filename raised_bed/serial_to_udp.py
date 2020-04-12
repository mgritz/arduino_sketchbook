#!/usr/bin/python3

import serial
import socket
import time
from collections import deque

import signal

leave = False

def signal_handler(sig, frame):
    global leave
    leave = True

sp = serial.Serial(port="/dev/ttyACM0", baudrate=9600, timeout=0)
rxbuf = deque()

udp_soc = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP socket

signal.signal(signal.SIGINT, signal_handler)

while not leave:
    # read from input
    while True:
        b = sp.read()
        if b == b'':
            break
        rxbuf.append(b)

    # parse input for one number
    while True:
        try:
            startidx = rxbuf.index(b'\n')
            endidx = rxbuf.index(b'\r', startidx)
        except ValueError:
            break
        if startidx is None or endidx is None or startidx + 1 > endidx:
            break

        numberstring = b''.join((list(rxbuf)[startidx + 1:endidx])).decode()

        message = "analog,input=A0 value={}".format(numberstring)

        # send stuff to UDP
        udp_soc.sendto(message.encode(), ("127.0.0.1", 8089))
        print(message)

        # remove parsed object from input stream
        try:
            while rxbuf.popleft() != '\r':
                pass
        except IndexError:
            pass

    time.sleep(1)

sp.close()
