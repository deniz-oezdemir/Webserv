#include "Client.hpp"
#include "Logger.hpp"
#include "macros.hpp"
#include <cstddef>
#include <string>
#include <unistd.h>
#include <vector>

Client::Client(int *fd)
{
	fd_ = fd;
	hasCompleteRequest_ = false;
	isChunked_ = false;
	isClosed_ = false;
}

Client::~Client(void)
{
	return;
}

bool Client::hasRequest(void)
{
	if (isClosed_)
	{
		Logger::log(Logger::ERROR)
			<< "Attemped to read from closed client! Client info: " << *this
			<< std::endl;
		return false;
	}

	if (hasCompleteRequest_)
	{
		Logger::log(Logger::ERROR)
			<< "Attemped to read from client that has "
			   "full request pending extraction! Client info: "
			<< *this << std::endl;
		return true;
	}

	// Read from file descriptor
	std::vector<char> buffer(BUFFER_SIZE);
	ssize_t bytesReadFromFd
		= read(*fd_, buffer.data(), buffer.size()); // TODO: remove magic number

	// If both buffer and file descriptor are empty connection is closed
	if (bytesReadFromFd == 0 && clientBuffer_.str().empty())
	{
		Logger::log(Logger::DEBUG)
			<< "Client fd and buffer empty." << std::endl;
		isClosed_ = true;
		return false;
	}

	clientBuffer_.write(buffer.data(), bytesReadFromFd);
	readClientBuffer_();
	return hasCompleteRequest_;
}

void Client::readClientBuffer_(void)
{
	while (clientBuffer_.str().empty() == false)
	{
		std::getline(clientBuffer_, requestStr_, '\n');
		requestStr_.append("\n");
		if (requestStr_.length() > BUFFER_SIZE)
		{
			Logger::log(Logger::INFO)
				<< "Client received request over default buffer size limit."
				<< std::endl;
			isClosed_ = true;
			return;
		}
		if (isCompleteRequest_() == true)
		{
			return;
		}
	}
}

bool Client::isCompleteRequest_(void)
{
	if (hasSizeIndicator() == false)
	{
		// If last line of requestStr_ is empty line it should be end of headers
		// with no body present and full request.
		if (requestStr_.find("\r\n\r\n") != std::string::npos)
		{
			hasCompleteRequest_ = true;
			return true;
		}
	}
	else
	{
		// First empty line signifies end of headers. If last line of
		// requestStr_ is second empty line, signifies end of body and full
		// request.
		if (requestStr_.find("\r\n\r\n") != std::string::npos)
		{
			hasCompleteRequest_ = true;
			return true;
		}
	}

	return false;
}

bool Client::hasSizeIndicator(void)
{
	return requestStr_.find("Content-Length") != std::string::npos ||
		   requestStr_.find("content-length") != std::string::npos ||
		   requestStr_.find("Transfer-Encoding") != std::string::npos ||
		   requestStr_.find("transfer-encoding") != std::string::npos;
}

std::string Client::extractRequestStr(void)
{
	std::string tmp = requestStr_;
	requestStr_.clear();
	hasCompleteRequest_ = false;

	return tmp;
}

bool Client::hasCompleteRequest(void) const
{
	return hasCompleteRequest_;
}

bool Client::isClosed(void) const
{
	return isClosed_;
}

int *Client::getFd(void) const
{
	return fd_;
}

std::ostream &operator<<(std::ostream &os, const Client &rhs)
{
	os << "Client: " << std::endl;
	os << "fd: " << *(rhs.getFd()) << std::endl;
	os << "has complete request: " << rhs.hasCompleteRequest() << std::endl;
	os << "is closed:" << rhs.isClosed() << std::endl;

	return os;
}
