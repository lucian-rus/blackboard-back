import socket
import time
from tkinter import *
import threading

clientsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
clientsocket.connect(('localhost', 8085))
    
# Variables
prevPoint = [0, 0]
currentPoint = [0, 0]

root = Tk()

root.title("tester")
root.geometry("1100x650")

root.resizable(False, False)
# Main Frame
frame2 = Frame(root, height=650, width=1100)
frame2.grid(row=1, column=0)

# Making a Canvas
canvas = Canvas(frame2, height=650, width=1100, bg="white")
canvas.grid(row=0, column=0)
canvas.place(relx=0.5, rely=0.5, anchor=CENTER)

def comm_thread():
    global prevPoint
    global currentPoint
    global canvas 
    global clientsocket

    while True:
        buf = clientsocket.recv(255)
        coord_list = list(buf)

        x = coord_list[2] << 8 | coord_list[3]
        y = coord_list[4] << 8 | coord_list[5]

        if x == 0 and y == 0:
            prevPoint = [0, 0]
        
        print(x, y)
        currentPoint = [x, y]

        if prevPoint != [0, 0]:
            canvas.create_polygon(
                prevPoint[0],
                prevPoint[1],
                currentPoint[0],
                currentPoint[1],
                fill="black",
                outline="black",
                width=1,
            )

        prevPoint = currentPoint

thread1 = threading.Thread(target=comm_thread)
thread1.start()

root.mainloop()