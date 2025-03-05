import socket
import time
from tkinter import *
import threading

clientsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
clientsocket.connect(('localhost', 8080))
connection_validation = [255, 4, 169, 185, 169, 185]
clientsocket.send(bytes(connection_validation))
    
######################################################################
prev_point = [0, 0]
current_point = [0, 0]
color = "white"

root = Tk()
root.title("tester RX")
root.geometry("1480x720")

root.resizable(False, False)
main_frame = Frame(root, height=720, width=1480)
main_frame.grid(row=0, column=0)

# canvas
canvas = Canvas(main_frame, height=720, width=1480, bg="black")
canvas.grid(row=0, column=0)
canvas.place(relx=0.5, rely=0.5, anchor=CENTER)

######################################################################
def exec_exit():
    # print("oi got called for close")
    clientsocket.close()
    root.quit()
    exit()

def update_coordinates(recv_list):
    x = recv_list[3] << 8 | recv_list[4]
    y = recv_list[5] << 8 | recv_list[6]
    return (x/2, y/2)

def update_color(recv_list):
    global color
    if recv_list[3] == 0:
        color = "red"
    elif recv_list[3] == 1:
        color = "blue"
    elif recv_list[3] == 2:
        color = "yellow"
    elif recv_list[3] == 3:
        color = "green"
    elif recv_list[3] == 4:
        color = "black"

def comm_thread():
    global prev_point
    global current_point
    global canvas 
    global clientsocket
    global color

    while True:
        buf = clientsocket.recv(7)
        recv_list = list(buf)
        print(recv_list)

        x, y = 0, 0
        if recv_list[2] == 255:
            exec_exit()
        elif recv_list[2] == 0:
            x, y = update_coordinates(recv_list)
        elif recv_list[2] == 1:
            update_color(recv_list)
        else:
            print("unhandled")

        if x == 0 and y == 0:
            prev_point = [0, 0]
        
        # print(x, y)
        current_point = [x, y]

        if prev_point != [0, 0]:
            canvas.create_polygon(
                prev_point[0],
                prev_point[1],
                current_point[0],
                current_point[1],
                fill=color,
                outline=color,
                width=1,
            )

        prev_point = current_point

######################################################################
thread1 = threading.Thread(target=comm_thread)
thread1.start()

root.mainloop()
