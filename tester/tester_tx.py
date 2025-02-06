import socket
import tkinter as tk

clientsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
clientsocket.connect(('localhost', 16080))

######################################################################
prev_point = [0, 0]
current_point = [0, 0]
color="black"

######################################################################
def paint(event):
    global prev_point
    global current_point
    global color

    x = event.x
    y = event.y

    xl = ((x >> 8) & 0xff)
    xr = (x & 0xff)
    yl = ((y >> 8) & 0xff)
    yr = (y & 0xff)
    coordinates_str = [255, 5, 0, xl, xr, yl, yr]
    print(coordinates_str)
    clientsocket.send(bytes(coordinates_str))

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
    if event.type == "5":
        prev_point = [0, 0]
        coordinates_str = [255, 5, 0, 0, 0, 0, 0]
        clientsocket.send(bytes(coordinates_str))


######################################################################
def set_color_to_red():
    global color
    color = "red"
    clientsocket.send(bytes([255, 2, 1, 0]))
def set_color_to_blue():
    global color
    color = "blue"
    clientsocket.send(bytes([255, 2, 1, 1]))
def set_color_to_yellow():
    global color
    color = "yellow"
    clientsocket.send(bytes([255, 2, 1, 2]))
def set_color_to_green():
    global color
    color = "green"
    clientsocket.send(bytes([255, 2, 1, 3]))
def set_color_to_black():
    global color
    color = "black"
    clientsocket.send(bytes([255, 2, 1, 4]))
def exec_exit():
    clientsocket.send(bytes([255, 1, 255]))
    clientsocket.close()
    exit()

######################################################################
root = tk.Tk()

root.title("tester TX")
root.geometry("1100x650")

root.resizable(False, False)
main_frame = tk.Frame(root)
main_frame.grid(row=0, column=0)

btn_frame = tk.Frame(main_frame, height=50, width=1100)
canvas_frame = tk.Frame(main_frame, height=600, width=1100)

btn_frame.grid(row=0, column=0)
canvas_frame.grid(row=1, column=0)

# button handlings
btn_red = tk.Button(btn_frame, text="Red", command=set_color_to_red)
btn_blue = tk.Button(btn_frame, text="Blue", command=set_color_to_blue)
btn_yellow = tk.Button(btn_frame, text="Yellow", command=set_color_to_yellow)
btn_green = tk.Button(btn_frame, text="Green", command=set_color_to_green)
btn_black = tk.Button(btn_frame, text="Black", command=set_color_to_black)
btn_exit = tk.Button(btn_frame, text="Exit", command=exec_exit)

btn_red.grid(row=0, column=0)
btn_blue.grid(row=0, column=1)
btn_yellow.grid(row=0, column=2)
btn_green.grid(row=0, column=3)
btn_black.grid(row=0, column=4)
btn_exit.grid(row=0, column=5)

# canvas
canvas = tk.Canvas(canvas_frame, height=600, width=1100, bg="white")
canvas.place(relx=0.5, rely=0.5, anchor="center")

# event bindings
canvas.bind("<B1-Motion>", paint)
canvas.bind("<ButtonRelease-1>", paint)
canvas.bind("<Button-1>", paint)

# update this to also have threading
root.mainloop()
