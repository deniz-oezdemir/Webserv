/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migmanu <jmanuelmigoya@gmail.com>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/28 15:26:45 by migmanu           #+#    #+#             */
/*   Updated: 2024/08/28 15:28:35 by migmanu          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"

Server::Server(int port) : port_(port)
{
	createSocket();
	bindSocket();
	listenSocket();
}

<<<<<<< HEAD
Server::Server(const Server& src)
=======
Server::Server(const Server &src)
>>>>>>> 70cf4b6 (basic TCP server functionality added to Server class)
{
	*this = src;
}

Server::~Server(void)
{
	close(serverFd_);
}

<<<<<<< HEAD
Server& Server::operator=(const Server& rhs)
=======
Server &Server::operator=(const Server &rhs)
>>>>>>> 70cf4b6 (basic TCP server functionality added to Server class)
{
	if (this != &rhs)
	{
		serverFd_ = rhs.serverFd_;
		serverAddr_ = rhs.serverAddr_;
		port_ = rhs.port_;
	}
	return *this;
}

void Server::createSocket()
{
	serverFd_ = socket(AF_INET, SOCK_STREAM, 0);
	if (serverFd_ == -1)
	{
		throw std::runtime_error("Failed to create socket");
	}
<<<<<<< HEAD

	// Set the socket to non-blocking mode
	int flags = fcntl(serverFd_, F_GETFL, 0);
	if (flags == -1 || fcntl(serverFd_, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		throw std::runtime_error("Failed to set non-blocking mode");
	}
=======
>>>>>>> 70cf4b6 (basic TCP server functionality added to Server class)
}

void Server::bindSocket()
{
	serverAddr_.sin_family = AF_INET;
	serverAddr_.sin_addr.s_addr = INADDR_ANY;
	serverAddr_.sin_port = htons(port_);

<<<<<<< HEAD
	// bind socket to server address
	if (bind(serverFd_, (struct sockaddr*)&serverAddr_, sizeof(serverAddr_)) <
		0)
=======
	if (bind(serverFd_, (struct sockaddr *)&serverAddr_, sizeof(serverAddr_)) < 0)
>>>>>>> 70cf4b6 (basic TCP server functionality added to Server class)
	{
		throw std::runtime_error("Failed to bind socket");
	}
}

void Server::listenSocket()
{
<<<<<<< HEAD
	// listen on the server socket for incoming connections, maximum length of
	// queue of pending connections 10
	if (listen(serverFd_, 10) < 0)
=======
	if (listen(serverFd_, 3) < 0)
>>>>>>> 70cf4b6 (basic TCP server functionality added to Server class)
	{
		throw std::runtime_error("Failed to listen on socket");
	}
}

<<<<<<< HEAD
void Server::handleClient(int clientFd)
{
	char buffer[1000];
	long bytesRead = read(clientFd, buffer, sizeof(buffer));
	if (bytesRead < 0)
	{
		if (errno != EWOULDBLOCK && errno != EAGAIN)
		{
			throw std::runtime_error("Failed to read from client");
		}
		return;
	}
	else if (bytesRead == 0)
	{
		// Client disconnected
		close(clientFd);
		return;
	}

	std::cout << "Hello from server. Your message was: " << buffer;

	std::string response = "Have a good day.\n";
	send(clientFd, response.c_str(), response.size(), 0);
}

void Server::start()
{
	// Initialize pollFds_ vector
	pollFds_.clear();

	// Create pollfd struct for the server socket and add it to the vector
	pollfd serverPollFd = {serverFd_, POLLIN, 0};
	pollFds_.push_back(serverPollFd);

	while (true)
	{
		int pollCount = poll(pollFds_.data(), pollFds_.size(), -1);
		if (pollCount == -1)
		{
			throw std::runtime_error("poll() failed");
		}

		for (size_t i = 0; i < pollFds_.size(); ++i)
		{
			// Check if fd is ready for reading
			if (pollFds_[i].revents & POLLIN)
			{
				// If the file descriptor is the server socket, accept a new
				// client connection
				if (pollFds_[i].fd == serverFd_)
				{
					acceptConnection();
				}
				else
				{
					// If the file descriptor is a client socket, handle client
					// I/O
					handleClient(pollFds_[i].fd);
				}
			}
		}
	}
}

void Server::acceptConnection()
{
	int addrLen = sizeof(serverAddr_);
	int clientFd =
		accept(serverFd_, (struct sockaddr*)&serverAddr_, (socklen_t*)&addrLen);
	if (clientFd < 0)
	{
		if (errno != EWOULDBLOCK && errno != EAGAIN)
		{
			throw std::runtime_error("Failed to accept connection");
		}
		return;
	}

	// Set the client socket to non-blocking mode
	int flags = fcntl(clientFd, F_GETFL, 0);
	if (flags == -1 || fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		throw std::runtime_error("Failed to set non-blocking mode for client");
	}

	pollfd clientPollFd = {clientFd, POLLIN, 0};
	pollFds_.push_back(clientPollFd);
=======
void Server::acceptConnection()
{
	int addrLen = sizeof(serverAddr_);
	int clientFd = accept(serverFd_, (struct sockaddr *)&serverAddr_, (socklen_t*)&addrLen);
	if (clientFd < 0)
	{
		throw std::runtime_error("Failed to accept connection");
	}
	handleClient(clientFd);
}

void Server::handleClient(int clientFd)
{
	char buffer[1000];
	long bytesRead = read(clientFd, buffer, 1000);
	if (bytesRead < 0)
	{
		throw std::runtime_error("Failed to read from client");
	}
	std::cout << "Hello from server. Your message was: " << buffer;

	//most likely send to be kept as has more options than write
	std::string responseSend = "Have a good day. (send)\n";
	send(clientFd, responseSend.c_str(), responseSend.size(), 0);
	std::string responseWrite = "What's up? (write)\n";
	write(clientFd, responseWrite.c_str(), responseWrite.size());

	close(clientFd);
}

void Server::start()
{
	acceptConnection();
>>>>>>> 70cf4b6 (basic TCP server functionality added to Server class)
}
