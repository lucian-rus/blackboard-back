import socket
import time

clientsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
clientsocket.connect(('localhost', 8080))

while True:
    clientsocket.send(b'\xff\x05hello')
    time.sleep(3)