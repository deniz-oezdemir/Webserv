#pragma once

#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

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
	int					serverFd_;
	int					port_;
	sockaddr_in			serverAddr_;
	std::vector<pollfd> pollFds_;

	void createSocket();
	void bindSocket();
	void listenSocket();
	void acceptConnection();
	void handleClient(int clientFd);
};
