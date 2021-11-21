#include "thread.h"
#include "socketserver.h"
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include "Semaphore.h"
#include <list>
#include <vector>
#include <thread>

using namespace Sync;

class SocketThread : public Thread
{
private:
    // reference to the connected socket
    Socket &socket;
    // byte array for received and sent data
    ByteArray data;
	// number of chat rooms
	int chatRoomNumber;
	// the port for the server
	int port;
    // reference to termination
    bool& terminate;
    // holder for SocketThread pointers
    std::vector<SocketThread*> &socketThreadsHolder;

public:
	SocketThread(Socket& socket, std::vector<SocketThread*> &clientSockThr, bool &terminate, int port) :
		socket(socket), socketThreadsHolder(clientSockThr), terminate(terminate), port(port)
	{}

    ~SocketThread()
    {
		this->terminationEvent.Wait();
	}

    Socket& GetSocket()
    {
        return socket;
    }

    const int GetChatRoom()
    {
        return chatRoomNumber;
    }

    virtual long ThreadMain()
    {
		// port number to a string value
		std::string stringPort = std::to_string(port);

		// generate semaphores on each socket thread with the string port number
		Semaphore clientBlock(stringPort);

		try {
			// read bytestream data
			socket.Read(data);

			// bytestream data to string
			std::string chatRoomString = data.ToString();
			chatRoomString = chatRoomString.substr(1, chatRoomString.size() - 1);
			chatRoomNumber = std::stoi(chatRoomString);
			std::cout << "Current chat room number: " << chatRoomNumber << std::endl;

			while(!terminate) {
				int socketResult = socket.Read(data);
				// if the client closes the socket, terminate this socket thread
				if (socketResult == 0)	break;

				std::string recv = data.ToString();
				if(recv == "shutdown\n") {
					// for mutual exclusion
					clientBlock.Wait();
					// iterate to select and erase this socket thread
					socketThreadsHolder.erase(std::remove(socketThreadsHolder.begin(), socketThreadsHolder.end(), this), socketThreadsHolder.end());
					// exit critical section
					clientBlock.Signal();

					// print termination message
					std::cout<< "A client is shutting off from our server. Erase client!" << std::endl;
					break;
				}

				// '/' appended as the first character to change the chat room number
				if (recv[0] == '/') {
					// splitting the received string
					std::string stringChat = recv.substr(1, recv.size() - 1);
				
					// parsing the integer after the forward slash character to get the chat room number
					chatRoomNumber = std::stoi(stringChat);
					std::cout << "A client has joined chat room " << chatRoomNumber << std::endl;
					continue;
				}

				// thread can entering the critical section
				clientBlock.Wait();
				for (int i = 0; i < socketThreadsHolder.size(); i++) {
					SocketThread *clientSocketThread = socketThreadsHolder[i];
					if (clientSocketThread->GetChatRoom() == chatRoomNumber)
					{
						Socket &clientSocket = clientSocketThread->GetSocket();
						ByteArray sendB(recv);
						clientSocket.Write(sendB);
					}
				}
				// exit the critical section
				clientBlock.Signal();
			}
		} 
		// to catch string-thrown exceptions
		catch(std::string &s) {
			std::cout << s << std::endl;
		}
		// to catch thrown exceptions and distinguish them in console
		catch(std::exception &e){
			std::cout << "A client has abruptly quit the app!" << std::endl;
		}

		// when a client exits the chat app
		std::cout << "A client has left the app!" << std::endl;
	}
};

class ServerThread : public Thread
{
private:
	// reference to socket server
    SocketServer &server;

    std::vector<SocketThread*> socketThrHolder;

	// socket port number
	int port;
	// chat room number
	int numberRooms;
	// boolean flag used for termination
    bool terminate = false;
    
public:
    ServerThread(SocketServer& server, int numberRooms, int port)
    : server(server), numberRooms(numberRooms), port(port)
    {}

    ~ServerThread()
    {
        // closing the client sockets
        for (auto thread : socketThrHolder)
        {
            try
            {
                // close the socket
                Socket& toClose = thread->GetSocket();
                toClose.Close();
            }
            catch (...)
            {
                // catch all exceptions
            }
        }
		std::vector<SocketThread*>().swap(socketThrHolder);
        terminate = true;
    }

    virtual long ThreadMain()
    {
        while (true)
        {
            try {
				// convert port number to a string
                std::string stringPortNum = std::to_string(port);
                // std::cout << "FlexWait/Natural blocking call on client!" <<std::endl;

				// main owner semaphore to block other semaphores by name
                Semaphore serverBlock(stringPortNum, 1, true);

				// the chat app front end receives number of chat rooms through the socket
                std::string chats = std::to_string(numberRooms) + '\n';

				// byte array conversion for chats
                ByteArray chats_conv(chats); 

                // wait for a client socket connection
                Socket sock = server.Accept();

				// send number of chats
                sock.Write(chats_conv);
                Socket* newConnection = new Socket(sock);

                // passing a reference to this pointer into a new socket thread
                Socket &socketReference = *newConnection;
                socketThrHolder.push_back(new SocketThread(socketReference, std::ref(socketThrHolder), terminate, port));
            }
			// catch string-thrown exception
            catch (std::string error)
            {
                std::cout << "ERROR: " << error << std::endl;
				// exit thread function
                return 1;
            }
			// to handle unexpected shutdown
			catch (TerminationException terminationException)
			{
				std::cout << "Server has shut down!" << std::endl;
				// exit with exception thrown
				return terminationException;
			}
        }
    }
};

int main(void) {
	// port number
    int port = 3005;

	// setting value of number of chat rooms for the server
    int rooms = 10;

    std::cout << "Chat App Server" << std::endl 
		<<"Type done to quit the server..." << std::endl;

	// creating the server
    SocketServer server(port);

	// thread to perform sever operations
    ServerThread st(server, rooms, port);

	// to wait for input to shutdown the server
	FlexWait cinWaiter(1, stdin);
	cinWaiter.Wait();
	std::cin.get();

	// shut down and clean up the server
	server.Shutdown();

    std::cout << "Bye!" << std::endl;
}