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

Server::Server(const Server& src)
{
	*this = src;
}

Server::~Server(void)
{
	close(serverFd_);
}

Server& Server::operator=(const Server& rhs)
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
}

void Server::bindSocket()
{
	serverAddr_.sin_family = AF_INET;
	serverAddr_.sin_addr.s_addr = INADDR_ANY;
	serverAddr_.sin_port = htons(port_);

	if (bind(serverFd_, (struct sockaddr*)&serverAddr_, sizeof(serverAddr_)) <
		0)
	{
		throw std::runtime_error("Failed to bind socket");
	}
}

void Server::listenSocket()
{
	if (listen(serverFd_, 3) < 0)
	{
		throw std::runtime_error("Failed to listen on socket");
	}
}

void Server::acceptConnection()
{
	int addrLen = sizeof(serverAddr_);
	int clientFd =
		accept(serverFd_, (struct sockaddr*)&serverAddr_, (socklen_t*)&addrLen);
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

	// most likely send to be kept as has more options than write
	std::string responseSend = "Have a good day. (send)\n";
	send(clientFd, responseSend.c_str(), responseSend.size(), 0);
	std::string responseWrite = "What's up? (write)\n";
	write(clientFd, responseWrite.c_str(), responseWrite.size());

	close(clientFd);
}

void Server::start()
{
	acceptConnection();
}
