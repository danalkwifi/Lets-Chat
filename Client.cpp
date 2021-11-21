#include "thread.h"
#include "socket.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>

using namespace Sync;

// thread to handle the connection to the server
class ClientThread : public Thread
{
private:
	// reference to the connected socket
	Socket& socket;

	// data to be sent
	ByteArray data;
	std::string data_str;
public:
	ClientThread(Socket& socket)
	: socket(socket)
	{}

	~ClientThread()
	{}

	virtual long ThreadMain()
	{
		int result = socket.Open();
		std::cout << "Enter your data (done to exit): ";
		std::cout.flush();

		// getting the data
		std::getline(std::cin, data_str);
		data = ByteArray(data_str);

		// writing to the server
		socket.Write(data);

		// getting the response
		socket.Read(data);
		data_str = data.ToString();
		std::cout << "Server Response: " << data_str << std::endl;
		return 0;
	}
};

int main(void)
{
	// welcome statement
	std::cout << "3313 Project Client" << std::endl;

	// creating the socket
	Socket socket("127.0.0.1", 3000);
	ClientThread clientThread(socket);
	while(1)
	{
		sleep(1);
	}
	socket.Close();

	return 0;
}
