#include "Logger.hpp"
#include "Server.hpp"
#include "ServerConfig.hpp"
#include "ServerInput.hpp"
#include "colors.hpp"
#include "macros.hpp"

#include <iostream>

Logger LOG;

int main(int argc, char *argv[])
{
	try
	{
		ServerInput input(argc, argv);
		if (input.hasThisFlag(ServerInput::HELP))
			std::cout << input.getHelpMessage() << std::endl;
		if (input.hasThisFlag(ServerInput::V_LITE) ||
			input.hasThisFlag(ServerInput::V_FULL))
			std::cout << input.getVersionMessage() << std::endl;
		ServerConfig config(input.getFilePath());
		config.parseFile(
			input.hasThisFlag(ServerInput::TEST),
			input.hasThisFlag(ServerInput::TEST_PRINT)
		);
		if (input.hasThisFlag(ServerInput::TEST) ||
			input.hasThisFlag(ServerInput::TEST_PRINT))
		{
			if (config.getIsConfigOK())
				std::cout << PURPLE "<WebServ> "
						  << RESET "Test finished: " << GREEN "Config OK!" RESET
						  << std::endl;
			if (input.hasThisFlag(ServerInput::TEST_PRINT))
				config.printConfig();
			return 0;
		}

		// Server server(PORT);
		// server.start();
	}
	catch (std::exception &e)
	{
		std::cerr << RED BOLD "Error:\t" RESET RED << e.what() << RESET
				  << std::endl;
	}
	/*
		//to be added to class
		//error checking to be added

		//create TCP/IP socket
		int serverFd = socket(AF_INET, SOCK_STREAM, 0);

		//set port
		sockaddr_in serverAddr;
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = INADDR_ANY;
		serverAddr.sin_port = htons(PORT);

		//bind socket
		bind(serverFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

		//listen
		listen(serverFd, QUEUE_SIZE);

		//get connection from queue
		int clientFd;
		int addrLen = sizeof(serverAddr);
		clientFd = accept(serverFd, (struct sockaddr *)&serverAddr,
	   (socklen_t*)&addrLen);

		//read
		char buffer[1000];
		long bytesRead;
		(void)bytesRead;
		bytesRead = read(clientFd, buffer, 1000);
		std::cout << "Hello from server. Your message was: " << buffer;

		//send
		std::string responseSend = "Have a good day. (send)\n";
		send(clientFd, responseSend.c_str(), responseSend.size(), 0);

		//write
		std::string responseWrite = "What's up? (write)\n";
		write(clientFd, responseWrite.c_str(), responseWrite.size());

		close(clientFd);
		close(serverFd);
	*/
	return 0;
}
