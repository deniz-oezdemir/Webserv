#include "ServerEngine.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Logger.hpp"
#include "RequestParser.hpp"
#include "utils.hpp"

#include <fcntl.h>
#include <unistd.h>

ServerEngine::ServerEngine() : numServers_(0) {}

ServerEngine::ServerEngine(
	std::vector<std::map<std::string, ConfigValue>> const &servers
)
	: numServers_(servers.size())
{
	this->servers_.reserve(this->numServers_);
	Logger::log(Logger::INFO)
		<< "Initializing the Server Engine with " << this->numServers_
		<< " servers..." << std::endl;
	for (size_t i = 0; i < this->numServers_; ++i)
	{
		this->servers_.push_back(Server(servers[i], i));
		this->servers_[i].initServer();
		Logger::log(Logger::INFO) << "Server " << i + 1 << " | " << "Listen:\t"
								  << this->servers_[i].getIPV4() << ':'
								  << this->servers_[i].getPort() << std::endl;
	}
}

ServerEngine::~ServerEngine() {}

void ServerEngine::initPollFds_(void)
{
	// Initialize pollFds_ vector
	pollFds_.clear();
	pollFds_.reserve(this->numServers_);

	Logger::log(Logger::DEBUG) << "Initializing pollFds_ vector" << std::endl;

	// Create pollfd struct for the server socket and add it to the vector
	for (size_t i = 0; i < this->numServers_; ++i)
	{
		pollfd serverPollFd = {servers_[i].getServerFd(), POLLIN, 0};
		pollFds_.push_back(serverPollFd);
	}
}

bool ServerEngine::isPollFdServer_(int &fd)
{
	for (size_t i = 0; i < this->numServers_; ++i)
	{
		if (fd == this->servers_[i].getServerFd())
			return true;
	}
	return false;
}

void ServerEngine::acceptConnection_(size_t &index)
{
	Logger::log(Logger::DEBUG) << "Accepting client connection on the server["
							   << index << ']' << std::endl;
	sockaddr_in serverAddr = this->servers_[index].getServerAddr();
	int			addrLen = sizeof(serverAddr);
	int			clientFd = accept(
		this->servers_[index].getServerFd(),
		(struct sockaddr *)&serverAddr,
		(socklen_t *)&addrLen
	);
	if (clientFd < 0)
	{
		if (errno != EWOULDBLOCK && errno != EAGAIN)
		{
			Logger::log(Logger::ERROR, true)
				<< "Failed to accept client connection: ("
				<< ft::toString(errno) << ") " << strerror(errno) << std::endl;
		}
		return;
	}
	Logger::log(Logger::DEBUG) << "Client connection accepted" << std::endl;

	// Set the client socket to non-blocking mode
	int flags = fcntl(clientFd, F_GETFL, 0);
	if (flags == -1)
	{
		Logger::log(Logger::ERROR, true)
			<< "Failed to get client socket flags: (" << ft::toString(errno)
			<< ") " << strerror(errno) << std::endl;
		close(clientFd);
		return;
	}
	if (fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		Logger::log(Logger::ERROR, true)
			<< "Failed to set client socket flags: (" << ft::toString(errno)
			<< ") " << strerror(errno) << std::endl;
		close(clientFd);
		return;
	}
	Logger::log(Logger::DEBUG)
		<< "Client socket set to non-blocking mode" << std::endl;

	pollfd clientPollFd = {clientFd, POLLIN, 0};
	pollFds_.push_back(clientPollFd);
	Logger::log(Logger::DEBUG)
		<< "Client connection added to pollFds_[" << index << "]" << std::endl;
}

// TODO: Create a dynamic buffer, read in a loop until the end of the request
void ServerEngine::handleClient_(size_t &index)
{
	Logger::log(Logger::DEBUG)
		<< "Handling client connection pollFds_[" << index << ']' << std::endl;

	static size_t const bufferSize = 4096;
	char				buffer[bufferSize];
	long bytesRead = read(this->pollFds_[index].fd, buffer, sizeof(buffer));
	if (bytesRead < 0)
	{
		if (errno != EWOULDBLOCK && errno != EAGAIN)
		{
			Logger::log(Logger::ERROR, true)
				<< "Failed to read from client: (" << ft::toString(errno)
				<< ") " << strerror(errno) << std::endl;
		}
		return;
	}
	else if (bytesRead == 0)
	{
		// Client disconnected
		Logger::log(Logger::DEBUG)
			<< "Client disconnected: pollFds_[" << index << "]"
			<< ", closing socket and deleting it from pollFds_" << std::endl;
		close(pollFds_[index].fd);
		pollFds_.erase(pollFds_.begin() + index);
		return;
	}

	Logger::log(Logger::DEBUG) << "Read " << bytesRead << " bytes" << std::endl;
	// Parse the request
	HttpRequest *request = NULL;
	try
	{
		std::string requestStr(buffer, bytesRead);
		request = new HttpRequest(RequestParser::parseRequest(requestStr));
		std::cout << "Hello from server. Your message was: " << buffer;

		std::cout << std::endl
				  << "Request received: " << std::endl
				  << *request << std::endl;
	}
	catch (std::exception &e)
	{
		std::cerr << RED BOLD "Error:\t" RESET RED << e.what() << RESET
				  << std::endl;
	}

	if (request != NULL)
	{
		std::string response = createResponse(*request);
		Logger::log(Logger::DEBUG) << "Sending response" << std::endl;
		int retCode
			= send(pollFds_[index].fd, response.c_str(), response.size(), 0);
		if (retCode < 0)
		{
			Logger::log(Logger::ERROR, true)
				<< "Failed to send response to client: (" << ft::toString(errno)
				<< ") " << strerror(errno) << std::endl;
			close(pollFds_[index].fd);
			pollFds_.erase(pollFds_.begin() + index);
			delete request;
			return;
		}
		else if (retCode == 0)
		{
			Logger::log(Logger::DEBUG)
				<< "Client disconnected pollFds_[" << index
				<< "], closing socket and deleting it from pollFds_"
				<< std::endl;
			close(pollFds_[index].fd);
			pollFds_.erase(pollFds_.begin() + index);
			delete request;
			return;
		}
		delete request;
	}
}

void ServerEngine::start()
{
	this->initPollFds_();
	Logger::log(Logger::INFO) << "Starting the Server Engine" << std::endl;
	while (true)
	{
		int pollCount = poll(pollFds_.data(), pollFds_.size(), -1);
		if (pollCount == -1)
		{
			Logger::log(Logger::ERROR, true)
				<< "poll() failed: (" << ft::toString(errno) << ") "
				<< strerror(errno) << std::endl;
			continue;
		}
		Logger::log(Logger::DEBUG)
			<< "poll() returned " << pollCount << " events" << std::endl;

		for (size_t i = 0; i < pollFds_.size(); ++i)
		{
			// Check if fd has some errors(pollerr, pollnval) or if the
			// connection was broken(pollhup)
			if (pollFds_[i].revents & (POLLERR | POLLHUP | POLLNVAL))
			{
				std::string error("");
				if (pollFds_[i].revents & POLLERR)
					error += "|POLLERR|";
				if (pollFds_[i].revents & POLLHUP)
					error += "|POLLHUP|";
				if (pollFds_[i].revents & POLLNVAL)
					error += "|POLLNVAL|";

				if (error == "|POLLHUP|")
					Logger::log(Logger::DEBUG)
						<< "Client disconnected on pollFds_[" << i
						<< "]: " << pollFds_[i].fd << std::endl;
				else
					Logger::log(Logger::ERROR, true)
						<< "Descriptor error on pollFds_[" << i
						<< "]: " << pollFds_[i].fd << " : (" << error << ") "
						<< std::endl;

				if (!this->isPollFdServer_(this->pollFds_[i].fd))
				{
					Logger::log(Logger::DEBUG)
						<< "Closing and deleting client socket: pollFds_[" << i
						<< "]: " << pollFds_[i].fd << std::endl;
					close(pollFds_[i].fd);
					this->pollFds_.erase(this->pollFds_.begin() + i);
				}
				// TODO: restart server descriptor from Server class and then
				// update pollFds_ if it is an serveFd (if it is worth it)
			}
			// Check if fd is ready for reading
			else if (pollFds_[i].revents & POLLIN)
			{
				Logger::log(Logger::DEBUG)
					<< "pollFds_[" << i << "] is ready for read" << std::endl;
				// If the file descriptor is the server socket, accept a new
				// client connection
				if (this->isPollFdServer_(pollFds_[i].fd))
				{
					acceptConnection_(i);
				}
				else
				{
					// If the file descriptor is a client socket, handle client
					// I/O
					handleClient_(i);
				}
			}
		}
	}
}

std::string ServerEngine::createResponse(const HttpRequest &request)
{
	if (request.getMethod() == "GET")
	{
		Logger::log(Logger::DEBUG) << "Handling GET" << std::endl;
		return handleGetRequest(request);
	}
	else if (request.getMethod() == "POST")
	{
		Logger::log(Logger::DEBUG) << "Handling POST" << std::endl;
		return handlePostRequest(request);
	}
	else if (request.getMethod() == "DELETE")
	{
		Logger::log(Logger::DEBUG) << "Handling DELETE" << std::endl;
		return handleDeleteRequest(request);
	}
	else
	{
		Logger::log(Logger::DEBUG) << "Handling Unallowed" << std::endl;
		return handleUnallowedRequest();
	}
}

// location hardcoded here, needed in HttpRequest request -> add to parsing?
// (map with target, location would be handy)
std::string ServerEngine::handleGetRequest(const HttpRequest &request)
{
	HttpResponse response;

	std::string location = "website";
	std::string filepath = location + request.getTarget();

	// Read the index.html file
	std::ifstream file(filepath);
	POST if (file.is_open())
	{
		Logger::log(Logger::DEBUG) << "Handling GET: file opened" << std::endl;
		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string body = buffer.str();
		// Set the response
		response.setStatusCode(200);
		response.setReasonPhrase("OK");
		response.setHeader("Server", "Webserv/0.1");
		response.setHeader("Date", createTimestamp());
		response.setHeader("Content-Type", "text/html; charset=UTF-8");
		response.setHeader("Content-Length", std::to_string(body.size()));
		response.setHeader("Connection", "keep-alive");
		response.setBody(body);
	}
	else
	{
		Logger::log(Logger::DEBUG)
			<< "Handling GET: file not found" << std::endl;
		response.setStatusCode(404);
		response.setReasonPhrase("Not Found");
		response.setHeader("Server", "Webserv/0.1");
		response.setHeader("Date", createTimestamp());
		response.setHeader("Content-Type", "text/html; charset=UTF-8");
		std::string body = "<html><body><h1>404 Not Found</h1></body></html>";
		response.setHeader("Content-Length", std::to_string(body.size()));
		response.setHeader("Connection", "keep-alive");
		response.setBody(body);
	}
	Logger::log(Logger::DEBUG) << "Handling GET: responding" << std::endl;
	return response.toString();
}

// to be implemented
std::string ServerEngine::handlePostRequest(const HttpRequest &request)
{
	(void)request;
	Logger::log(Logger::DEBUG) << "Handling POST: responding" << std::endl;
	return "POST test\n";
}

// to be implemented
std::string ServerEngine::handleDeleteRequest(const HttpRequest &request)
{
	(void)request;
	Logger::log(Logger::DEBUG) << "Handling DELETE: responding" << std::endl;
	return "DELETE test\n";
}

// already handled in checkMethod() but should it be handled with a response?
// needs testing when usage implemented
std::string ServerEngine::handleUnallowedRequest()
{
	HttpResponse response;
	response.setStatusCode(405);
	response.setReasonPhrase("Method Not Allowed");
	response.setHeader("Content-Type", "text/html; charset=UTF-8");
	response.setHeader("Server", "Webserv/0.1");
	response.setHeader("Date", createTimestamp());
	std::string body
		= "<html><body><h1>405 Method Not Allowed</h1></body></html>";
	response.setHeader("Content-Length", std::to_string(body.size()));
	response.setHeader("Connection", "keep-alive");
	response.setBody(body);
	return response.toString();
}

std::string ServerEngine::createTimestamp()
{
	time_t	   now = time(0);
	struct tm *tstruct = localtime(&now);
	if (tstruct == nullptr)
	{
		throw std::runtime_error("Failed to get local time");
	}

	char buf[80];
	strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", tstruct);
	return std::string(buf);
}
