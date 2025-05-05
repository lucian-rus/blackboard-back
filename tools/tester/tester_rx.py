import socket
import time
from tkinter import *
import threading
import sys

clientsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
while True:
    try:
        clientsocket.connect((sys.argv[1], 8080))
        connection_validation = [255, 4, 169, 185, 169, 185]
        clientsocket.send(bytes(connection_validation))
        break
    except:
        print("waiting...")
    
######################################################################
prev_point = [0, 0]
current_point = [0, 0]
color = "white"
current_color_counter = 0
current_line_width = 1

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
    global current_color_counter

    current_color_counter = current_color_counter + 1
    if 5 == current_color_counter:
        current_color_counter = 0

    if current_color_counter == 0:
        color = "white"
    elif current_color_counter == 1:
        color = "blue"
    elif current_color_counter == 2:
        color = "yellow"
    elif current_color_counter == 3:
        color = "green"
    elif current_color_counter == 4:
        color = "red"

def clear_canvas():
    global canvas
    canvas.delete("all")

def update_line_width():
    global current_line_width

    current_line_width += 1
    if 8 == current_line_width:
        current_line_width = 1

def comm_thread():
    global prev_point
    global current_point
    global canvas 
    global clientsocket
    global color
    global current_line_width

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
        elif recv_list[2] == 2:
            clear_canvas()
        elif recv_list[2] == 3:
            update_line_width()
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
                width=(2 * current_line_width),
            )

        prev_point = current_point

######################################################################
thread1 = threading.Thread(target=comm_thread)
thread1.start()

root.mainloop()
