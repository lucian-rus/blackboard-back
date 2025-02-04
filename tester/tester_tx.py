import socket
import time
from tkinter import *

clientsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
clientsocket.connect(('localhost', 8080))

# Variables
prevPoint = [0, 0]
currentPoint = [0, 0]

# Paint Function
def paint(event):
    global prevPoint
    global currentPoint

    x = event.x
    y = event.y

    xl = ((x >> 8) & 0xff)
    xr = (x & 0xff)
    yl = ((y >> 8) & 0xff)
    yr = (y & 0xff)
    coordinates_str = [255, 4, xl, xr, yl, yr]
    print(coordinates_str)
    clientsocket.send(bytes(coordinates_str))

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

    if event.type == "5":
        prevPoint = [0, 0]


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
# Event Binding
canvas.bind("<B1-Motion>", paint)
canvas.bind("<ButtonRelease-1>", paint)
canvas.bind("<Button-1>", paint)

root.mainloop()

# while True:
#     clientsocket.send(b'\xff\x05hello')
#     time.sleep(3)