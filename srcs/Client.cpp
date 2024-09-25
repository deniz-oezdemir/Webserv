#include "Client.hpp"
#include "Logger.hpp"
#include "macros.hpp"
#include "utils.hpp"
#include <cstddef>
#include <cstdlib>
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
	ssize_t			  bytesReadFromFd
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
	size_t headerEndPos = requestStr_.find("\r\n\r\n");
	if (headerEndPos != std::string::npos)
	{
		if (!hasSizeIndicator_())
		{
			// No body, request ends at the first \r\n\r\n
			hasCompleteRequest_ = true;
			extractExtraChars_(headerEndPos + 4);
			return true;
		}
		else
		{
			// Request has a body, check if the body is fully received
			if (isBodyFullyReceived_(headerEndPos))
			{
				hasCompleteRequest_ = true;
				return true;
			}
		}
	}

	return false;
}

bool Client::isBodyFullyReceived_(size_t headerEndPos)
{
	size_t bodyStartPos = headerEndPos + 4;
	if (requestStr_.find("Content-Length") != std::string::npos)
	{
		size_t contentLengthPos = requestStr_.find("Content-Length");
		size_t contentLengthEndPos = requestStr_.find("\r\n", contentLengthPos);
		std::string contentLengthStr = requestStr_.substr(
			contentLengthPos + 15, contentLengthEndPos - (contentLengthPos + 15)
		);
		size_t contentLength = std::atoi(contentLengthStr.c_str());
		if (requestStr_.size() >= bodyStartPos + contentLength)
		{
			extractExtraChars_(bodyStartPos + contentLength);
			return true;
		}
	}
	else if (requestStr_.find("Transfer-Encoding") != std::string::npos)
	{
		// Handle chunked transfer encoding
		size_t chunkEndPos = requestStr_.find("0\r\n\r\n", bodyStartPos);
		if (chunkEndPos != std::string::npos)
		{
			extractExtraChars_(chunkEndPos + 5);
			return true;
		}
	}
	return false;
}

void Client::extractExtraChars_(size_t pos)
{
	std::string extraChars = requestStr_.substr(pos);
	requestStr_ = requestStr_.substr(0, pos);
	clientBuffer_.str(extraChars + clientBuffer_.str());
	clientBuffer_.seekg(0, std::ios::beg); // Reset the get pointer
}

bool Client::hasSizeIndicator_(void)
{
	return ft::caseInsensitiveFind(requestStr_, "Content-Length")
		   || ft::caseInsensitiveFind(requestStr_, "Transfer-Encoding");
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
