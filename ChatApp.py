# Group 3; Stella, Alexandria, Dana, Temi

# import statements
from socket import AF_INET, socket, SOCK_STREAM
from threading import Thread
import tkinter
from tkinter import messagebox

# function to receive message; must be threaded
def receiveMessage():
    while True:
        try:
            msg = client_socket.recv(BUFFER_SIZE).decode("utf8")
            msg_list.insert(tkinter.END, msg)
            msg_list.see(tkinter.END)
        except OSError:
            break

# function to send message
# binders to pass the event
def send(event=None):  
    msg = my_msg.get()
    # clears the text field
    my_msg.set("")      
    global current_room
    # if user sends the message 'quit'
    if msg == "{quit}": 
        # message indicating the quitting
        client_socket.send(bytes(my_username.get() + " has closed the Chat App!", "utf8"))
        # close the socket
        client_socket.close()
        top.quit()
        return
    client_socket.send(bytes(my_username.get() + ": " + msg, "utf8"))


# sending quit message to the server
def on_closing(event=None):
    # sending server quit message
    my_msg.set("{quit}")
    send()

# changing the chat room
def change_room():
    global current_room
    current_room = ((chatRoomSelected.get()).split(' '))[2]
    client_socket.send(bytes("/" + current_room, "utf8"))
    msg_list.delete(0, tkinter.END)
    msg_list.insert(tkinter.END, "You are now in room " + str(current_room))
    msg_list.see(tkinter.END)

# declaring global variables
number_of_rooms = 0
current_room = 0

# creating the user side of the app using tkinter
top = tkinter.Tk()
top.title("Let's Chat App")

messages_frame = tkinter.Frame(top)
# variable for the messages to be sent
my_msg = tkinter.StringVar()
my_msg.set("")
# variable for the usernames to be sent
my_username = tkinter.StringVar()
my_username.set("")

# to see the previous messages
scrollbar = tkinter.Scrollbar(messages_frame)  
# contains the messages
msg_list = tkinter.Listbox(messages_frame, bg="grey", fg="black", font='helvetica', height=20, width=70, yscrollcommand=scrollbar.set)
scrollbar.pack(side=tkinter.RIGHT, fill=tkinter.Y)
msg_list.pack(side=tkinter.LEFT, fill=tkinter.BOTH)
msg_list.pack()
messages_frame.pack()

# for sending messages
username_label = tkinter.Label(top, text="Username: ", fg="yellow", bg="black", font='helvetica')
username_label.pack()
username_field = tkinter.Entry(top, textvariable=my_username)
username_field.pack()

message_label = tkinter.Label(top, text="Chat here: ", fg="yellow", bg="black", font='helvetica')
message_label.pack()
entry_field = tkinter.Entry(top, textvariable=my_msg, width=30)
entry_field.bind("<Return>", send)
entry_field.pack()
send_button = tkinter.Button(top, text="Send!", command=send)
send_button.pack()

def on_closing():
    if messagebox.askokcancel("Quit", "Do you want to exit?"):
        top.destroy()

top.protocol("WM_DELETE_WINDOW", on_closing)

# socket with given parameters
HOST = "192.168.0.37"
PORT = 3005
BUFFER_SIZE = 1024
ADDR = (HOST, PORT)

# create socket connection
client_socket = socket(AF_INET, SOCK_STREAM)
client_socket.connect(ADDR)

# server response of number of rooms available
# generating drop down list
first_msg = client_socket.recv(BUFFER_SIZE).decode("utf8")
number_of_rooms = int(first_msg)
chatRoomSelected = tkinter.StringVar(top)
chatRoomSelected.set("Select a Chat Room Here!")
rooms_list = []
for i in range(number_of_rooms):
    rooms_list.append("Chat Room " + str(i + 1))

# for showing the drop down list
chat_rooms = tkinter.OptionMenu(top, chatRoomSelected, *rooms_list)
chat_rooms.pack()
change_button = tkinter.Button(top, text="Change Room!", command=change_room, bg="blue", fg="white", font='helvetica')
change_button.pack()

# threads for received messages
receive_thread = Thread(target=receiveMessage)
receive_thread.start()

# the client cannot resize the window
top.resizable(width=False, height=False)    
tkinter.mainloop()
