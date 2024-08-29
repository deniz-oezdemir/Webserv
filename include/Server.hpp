/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migmanu <jmanuelmigoya@gmail.com>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/28 15:28:13 by migmanu           #+#    #+#             */
/*   Updated: 2024/08/28 15:28:14 by migmanu          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdexcept>
#include <iostream>

class Server
{
	public:
		Server(int port);
		Server(const Server &src);
		~Server(void);

		Server &operator=(const Server &rhs);

		void start();

	protected:

	private:
		int	serverFd_;
		sockaddr_in serverAddr_;
		int port_;

		void createSocket();
		void bindSocket();
		void listenSocket();
		void acceptConnection();
		void handleClient(int clientFd);
};
