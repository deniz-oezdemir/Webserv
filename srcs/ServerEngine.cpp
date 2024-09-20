#include "ServerEngine.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Logger.hpp"
#include "request_parser/RequestParser.hpp"
#include "utils.hpp"

#include <fcntl.h>
#include <fstream>
#include <unistd.h>

ServerEngine::ServerEngine(
	// clang-format off
	std::vector<std::map<std::string, ConfigValue> > const &servers
	// clang-format on
)
	: numServers_(servers.size())
{
	Logger::log(Logger::INFO)
		<< "Initializing the Server Engine with " << this->numServers_
		<< " servers..." << std::endl;
	size_t globalServerIndex(0);
	for (size_t serverIndex = 0; serverIndex < this->numServers_; ++serverIndex)
	{
		globalServerIndex
			+= servers[serverIndex].at("listen").getMapValue("port").size();
	}
	this->servers_.reserve(globalServerIndex);
	globalServerIndex = 0;
	for (size_t serverIndex = 0; serverIndex < this->numServers_; ++serverIndex)
	{
		this->initServer_(servers[serverIndex], serverIndex, globalServerIndex);
	}
	this->totalServerInstances_ = globalServerIndex;
}

ServerEngine::~ServerEngine()
{
	Logger::log(Logger::DEBUG)
		<< "Shutting down the server engine" << std::endl;
	for (size_t i = 0; i < pollFds_.size(); ++i)
	{
		if (!this->isPollFdServer_(pollFds_[i].fd) && pollFds_[i].fd != -1)
		{
			close(pollFds_[i].fd);
			pollFds_[i].fd = -1;
		}
	}
}

void ServerEngine::initServer_(
	std::map<std::string, ConfigValue> const &serverConfig,
	size_t									 &serverIndex,
	size_t									 &globalServerIndex
)
{
	size_t listenSize = serverConfig.at("listen").getMapValue("port").size();
	for (size_t listenIndex = 0; listenIndex < listenSize; ++listenIndex)
	{
		this->servers_.push_back(
			Server(serverConfig, globalServerIndex, listenIndex)
		);
		this->servers_[globalServerIndex].init();
		Logger::log(Logger::INFO)
			<< "Server " << serverIndex + 1 << "[" << listenIndex << "] "
			<< "| " << "Listen: " << this->servers_[globalServerIndex].getIPV4()
			<< ':' << this->servers_[globalServerIndex].getPort() << std::endl;
		++globalServerIndex;
	}
}

/* The serverFd is closed and initialized again, than the pollFds_ vector is
updated with the new file descriptor. */
void ServerEngine::restartServer_(size_t &index)
{
	Logger::log(Logger::INFO)
		<< "Restarting server[" << index << "]" << std::endl;
	this->servers_[index].resetServer();
	pollfd serverPollFd = {servers_[index].getServerFd(), POLLIN, 0};
	pollFds_[index] = serverPollFd;
	Logger::log(Logger::INFO)
		<< "Server[" << index << "] restarted" << std::endl;
}

void ServerEngine::initPollFds_(void)
{
	// Initialize pollFds_ vector
	pollFds_.clear();
	pollFds_.reserve(this->totalServerInstances_);

	Logger::log(Logger::DEBUG) << "Initializing pollFds_ vector" << std::endl;

	// Create pollfd struct for the server socket and add it to the vector
	for (size_t i = 0; i < this->totalServerInstances_; ++i)
	{
		pollfd serverPollFd = {servers_[i].getServerFd(), POLLIN, 0};
		pollFds_.push_back(serverPollFd);
	}
}

bool ServerEngine::isPollFdServer_(int &fd)
{
	for (size_t i = 0; i < this->totalServerInstances_; ++i)
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
		Logger::log(Logger::DEBUG) << "Request received:\n\nBuffer:\n"
								   << requestStr << "Request:\n"
								   << *request << std::flush;
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

void ServerEngine::pollFdError_(size_t &index)
{
	std::string error("");
	if (pollFds_[index].revents & POLLERR)
		error += "|POLLERR|";
	if (pollFds_[index].revents & POLLHUP)
		error += "|POLLHUP|";
	if (pollFds_[index].revents & POLLNVAL)
		error += "|POLLNVAL|";

	if (error == "|POLLHUP|")
	{
		Logger::log(Logger::DEBUG)
			<< "Client disconnected on pollFds_[" << index
			<< "]: " << pollFds_[index].fd << std::endl;
	}
	else
	{
		Logger::log(Logger::ERROR, true)
			<< "Descriptor error on pollFds_[" << index
			<< "]: " << pollFds_[index].fd << " : (" << error << ") "
			<< std::endl;
	}
	if (!this->isPollFdServer_(this->pollFds_[index].fd))
	{
		Logger::log(Logger::DEBUG)
			<< "Closing and deleting client socket: pollFds_[" << index
			<< "]: " << pollFds_[index].fd << std::endl;
		close(pollFds_[index].fd);
		this->pollFds_.erase(this->pollFds_.begin() + index);
	}
	else
	{
		restartServer_(index);
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
				pollFdError_(i);
			}
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
	int serverIndex = this->findServer_(request.getHost(), request.getPort());
	if (serverIndex == -1)
	{
		Logger::log(Logger::DEBUG) << "Server not found" << std::endl;
		return this->handleDefaultErrorResponse(404, "Not Found", true);
	}
	if (request.getMethod() == "GET")
	{
		Logger::log(Logger::DEBUG) << "Handling GET" << std::endl;
		return handleGetRequest(request, this->servers_[serverIndex]);
	}
	else if (request.getMethod() == "POST")
	{
		Logger::log(Logger::DEBUG) << "Handling POST" << std::endl;
		return handlePostRequest(request, this->servers_[serverIndex]);
	}
	else if (request.getMethod() == "DELETE")
	{
		Logger::log(Logger::DEBUG) << "Handling DELETE" << std::endl;
		return handleDeleteRequest(request, this->servers_[serverIndex]);
	}
	else
	{
		Logger::log(Logger::DEBUG) << "Method not implemented" << std::endl;
		return this->handleDefaultErrorResponse(501, "Not Implemented", true);
	}
}

int ServerEngine::findServer_(
	std::string const	 &host,
	unsigned short const &port
)
{
	for (size_t i = 0; i < this->totalServerInstances_; ++i)
	{
		std::vector<std::string> const &serverNames
			= this->servers_[i].getServerName();
		for (size_t j = 0; j < serverNames.size(); ++j)
		{
			if (serverNames[j] == host && this->servers_[i].getPort() == port)
				return i;
		}
	}
	return -1;
}

std::string
ServerEngine::handleGetRequest(const HttpRequest &request, Server const &server)
{
	std::string uri = request.getUri();
	std::string rootdir = server.getRoot();
	std::string filepath = rootdir + uri;

	// clang-format off
	std::map<std::string, std::vector<std::string> > location;
	// clang-format on
	if (server.isThisLocation(uri))
		location = server.getThisLocation(uri);
	else
		return this->handleDefaultErrorResponse(404);

	// clang-format off
	std::map<std::string, std::vector<std::string> >::const_iterator it
		= location.find("return");
	// clang-format on

	if (it != location.end() && it->second.size() != 0)
	{
		Logger::log(Logger::DEBUG)
			<< "GET: return directive (redirect)" << std::endl;
		return this->handleReturnDirective_(it->second);
	}

	HttpResponse response;
	// TODO: replace below with readFile
	std::ifstream file(filepath);
	if (file.is_open())
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

		// TODO: replace hardcoded /404.html with file from config? check
		// with Seba if needed
		std::string body = readFile(rootdir + "/404.html");

		response.setHeader("Content-Length", std::to_string(body.size()));
		response.setHeader("Connection", "keep-alive");
		response.setBody(body);
	}
	Logger::log(Logger::DEBUG) << "Handling GET: responding" << std::endl;
	return response.toString();
}

// TODO: implement check for file/directory, coordinate with Seba
std::string ServerEngine::handleDeleteRequest(
	const HttpRequest &request,
	Server const	  &server
)
{
	(void)request;
	(void)server;
	std::cout << request << std::endl;

	// Get root path from config of server
	std::string rootdir = servers_[0].getRoot();
	// Combine root path with uri from request
	std::string filepath = rootdir + request.getUri();

	Logger::log(Logger::DEBUG) << "Filepath: " << filepath << std::endl;

	HttpResponse response;

	if (remove(filepath.c_str()) == 0)
	{
		std::string body
			= "<!DOCTYPE html>\n<html>\n<head><title>200 OK</title></head>\n"
			  "<body><h1>File deleted.</h1></body>\n</html>\n";
		response.setStatusCode(200);
		response.setReasonPhrase("OK");
		response.setHeader("Server", "Webserv/0.1");
		response.setHeader("Date", createTimestamp());
		response.setHeader("Content-Type", "text/html; charset=UTF-8");
		response.setHeader("Content-Length", std::to_string(body.size()));
		response.setHeader("Connection", "keep-alive");
		response.setBody(body);
		Logger::log(Logger::INFO)
			<< "DELETE " << request.getUri() << " -> 200 OK" << std::endl;
	}
	else
	{
		std::string body = readFile(rootdir + "/404.html");
		response.setStatusCode(404);
		response.setReasonPhrase("Not Found");
		response.setHeader("Server", "Webserv/0.1");
		response.setHeader("Date", createTimestamp());
		response.setHeader("Content-Type", "text/html; charset=UTF-8");
		response.setHeader("Content-Length", std::to_string(body.size()));
		response.setHeader("Connection", "keep-alive");
		response.setBody(body);
		Logger::log(Logger::INFO) << "DELETE " << request.getUri()
								  << " -> 404 Not Found" << std::endl;
	}

	Logger::log(Logger::DEBUG) << "Handling DELETE: responding" << std::endl;
	return response.toString();
}

// TODO: implement
std::string ServerEngine::handlePostRequest(
	const HttpRequest &request,
	Server const	  &server
)
{
	(void)request;
	(void)server;
	Logger::log(Logger::DEBUG) << "Handling POST: responding" << std::endl;
	return "POST test\n";
}

// commented out similar functionality via exception by
// RequestParser::checkMethod_() as it should be handled with a 501 response to
// the client
std::string ServerEngine::handleNotImplementedRequest()
{
	// Get root path from config of server
	std::string rootdir = servers_[0].getRoot();

	Logger::log(Logger::DEBUG) << "Rootdir: " << rootdir << std::endl;
	HttpResponse response;
	response.setStatusCode(501);
	response.setReasonPhrase("Not Implemented");
	response.setHeader("Server", "Webserv/0.1");
	response.setHeader("Date", createTimestamp());
	response.setHeader("Content-Type", "text/html; charset=UTF-8");

	// TODO: replace hardcoded /404.html with file from config? check
	// with Seba if needed
	std::string body = readFile(rootdir + "/501.html");

	response.setHeader("Content-Length", std::to_string(body.size()));
	// nginx typically closes the TCP connection after sending a 501 response,
	// do we want to implement it?
	response.setHeader("Connection", "close");
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

std::string ServerEngine::readFile(const std::string &filePath)
{
	std::ifstream file(filePath);
	if (!file.is_open())
	{
		Logger::log(Logger::ERROR, true)
			<< "Failed to open file: " << filePath << std::endl;
		return "";
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	if (buffer.str().empty())
	{
		Logger::log(Logger::ERROR, true)
			<< "File is empty or could not be read: " << filePath << std::endl;
	}
	return buffer.str();
}

std::string
ServerEngine::handleDefaultErrorResponse(int statusCode, bool closeConnection)
{
	Logger::log(Logger::DEBUG)
		<< "Handling default error response: [" << statusCode << "] "
		<< getStatusCodeReason(statusCode) << std::endl;

	HttpResponse response;
	response.setStatusCode(statusCode);
	response.setReasonPhrase(getStatusCodeReason(statusCode));
	response.setHeader("Server", "Webserv/0.1");
	response.setHeader("Date", createTimestamp());
	response.setHeader("Content-Type", "text/html; charset=UTF-8");

	std::string body = readFile("www/" + std::to_string(statusCode) + ".html");
	response.setHeader("Content-Length", std::to_string(body.size()));
	if (closeConnection)
		response.setHeader("Connection", "close");
	response.setBody(body);
	return response.toString();
}

std::string ServerEngine::handleReturnDirective_(
	std::vector<std::string> const &returnDirective
)
{
	HttpResponse response;
	int			 statusCode = 301;

	if (returnDirective.size() == 2)
	{
		statusCode = std::atoi(returnDirective[0].c_str());
		response.setHeader("Location", returnDirective[1]);
	}
	else if (returnDirective.size() == 1)
	{
		response.setHeader("Location", returnDirective[0]);
	}

	Logger::log(Logger::DEBUG)
		<< "Handling return directive: [" << statusCode << "] "
		<< getStatusCodeReason(statusCode)
		<< "To: " << response.getHeader("Location") << std::endl;

	response.setStatusCode(statusCode);
	response.setReasonPhrase(getStatusCodeReason(statusCode));
	response.setHeader("Server", "Webserv/0.1");
	response.setHeader("Date", createTimestamp());
	response.setHeader("Content-Type", "text/html; charset=UTF-8");
	response.setHeader("Content-Length", "0");

	return response.toString();
}

std::string const &getStatusCodeReason(unsigned int code)
{
	static std::map<int, std::string> httpStatusCodes;
	if (httpStatusCodes.empty())
	{
		httpStatusCodes[200] = "OK";
		httpStatusCodes[201] = "Created";
		httpStatusCodes[202] = "Accepted";
		httpStatusCodes[204] = "No Content";
		httpStatusCodes[301] = "Moved Permanently";
		httpStatusCodes[302] = "Found";
		httpStatusCodes[303] = "See Other";
		httpStatusCodes[304] = "Not Modified";
		httpStatusCodes[400] = "Bad Request";
		httpStatusCodes[401] = "Unauthorized";
		httpStatusCodes[403] = "Forbidden";
		httpStatusCodes[404] = "Not Found";
		httpStatusCodes[405] = "Method Not Allowed";
		httpStatusCodes[408] = "Request Timeout";
		httpStatusCodes[411] = "Length Required";
		httpStatusCodes[413] = "Payload Too Large";
		httpStatusCodes[414] = "URI Too Long";
		httpStatusCodes[415] = "Unsupported Media Type";
		httpStatusCodes[500] = "Internal Server Error";
		httpStatusCodes[501] = "Not Implemented";
		httpStatusCodes[505] = "HTTP Version Not Supported";
	}
	if (httpStatusCodes.find(code) == httpStatusCodes.end())
		return httpStatusCodes[500];
	return httpStatusCodes[code];
}
