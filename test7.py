#!/usr/bin/env python
#coding:utf-8

#test7 对应的客户端
import socket

s = socket.socket()
s.connect(("10.211.55.3", 9981))
s.send("test")
print s.recv(1024)
s.close()
