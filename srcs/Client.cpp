#include "Client.hpp"
#include "Logger.hpp"
#include "macros.hpp"
#include "utils.hpp"
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <vector>

Client::Client(int pollFd)
{
	pollFd_ = pollFd;
	hasCompleteRequest_ = false;
	isChunked_ = false;
	isClosed_ = false;
}

Client::~Client(void)
{
	reset_();

	return;
}

Client::Client(const Client &src)
{
	*this = src;
}

Client &Client::operator=(const Client &rhs)
{
	pollFd_ = rhs.pollFd_;
	requestStr_ = rhs.requestStr_;
	isChunked_ = rhs.isChunked_;
	hasCompleteRequest_ = rhs.hasCompleteRequest_;
	isClosed_ = rhs.isClosed_;

	// Clear the existing content of clientBuffer_ and copy the content from rhs
	clientBuffer_.str("");
	clientBuffer_.clear();
	clientBuffer_ << rhs.clientBuffer_.str();

	return *this;
}

/**
 * @brief Checks if there is a complete request from the client.
 *
 * Reads data from the file descriptor into the client buffer and processes
 * it to determine if a complete request has been received.
 *
 * @return true if a complete request has been received, false otherwise.
 */
bool Client::hasRequestReady(void)
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
	ssize_t bytesReadFromFd = read(pollFd_, buffer.data(), buffer.size());

	if (bytesReadFromFd < 0)
	{
		if (errno != EWOULDBLOCK && errno != EAGAIN)
		{
			Logger::log(Logger::ERROR, true)
				<< "Failed to read from client: (" << ft::toString(errno)
				<< ") " << strerror(errno) << std::endl;
		}
		isClosed_ = true;
		return false;
	}
	else if (bytesReadFromFd == 0)
	{
		if (errno != EWOULDBLOCK && errno != EAGAIN)
		{
			Logger::log(Logger::INFO, true)
				<< "Client disconnected: " << *this << std::endl;
		}
		isClosed_ = true;
		return false;
	}

	clientBuffer_.write(buffer.data(), bytesReadFromFd);
	readClientBuffer_();
	return hasCompleteRequest_;
}

std::string Client::extractRequestStr(void)
{
	std::string tmp = requestStr_;
	reset_();

	return tmp;
}

int Client::getFd(void) const
{
	return pollFd_;
}

bool Client::isClosed(void) const
{
	return isClosed_;
}

bool Client::isChunked(void) const
{
	return isChunked_;
}

/**
 * @brief Reads data from the client buffer and processes it.
 *
 * Reads lines from the client buffer into the request string and checks if
 * a complete request has been received.
 */
void Client::readClientBuffer_(void)
{
	while (clientBuffer_.str().empty() == false)
	{
		std::string line;
		std::getline(clientBuffer_, line, '\n');
		requestStr_ += line + "\n";
		if (requestStr_.length() > BUFFER_SIZE)
		{
			Logger::log(Logger::INFO)
				<< "Client sent request over default buffer size limit."
				<< "\nrequestStr_.length(): " << requestStr_.length()
				<< "\nrequesStr_: " << requestStr_ << std::endl;
			isClosed_ = true;
			return;
		}
		if (isCompleteRequest_() == true)
		{
			return;
		}
	}
}

/**
 * @brief Checks if the current request is complete.
 *
 * Determines if the request string contains a complete HTTP request by
 * checking for the end of headers and the presence of a body if indicated
 * by the headers.
 *
 * @return true if the request is complete, false otherwise.
 */
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
				if (isChunked_ == false)
				{
					hasCompleteRequest_ = true;
					return true;
				}
				// TODO: extract chunked body
			}
		}
	}

	return false;
}

/**
 * @brief Checks if the body of the request is fully received.
 *
 * Determines if the body of the request has been fully received based on
 * the Content-Length or Transfer-Encoding headers.
 *
 * @param headerEndPos The position in the request string where the headers end.
 * @return true if the body is fully received, false otherwise.
 */
bool Client::isBodyFullyReceived_(size_t headerEndPos)
{
	size_t bodyStartPos = headerEndPos + 4;
	if (ft::caseInsensitiveFind(requestStr_, "Content-Length"))
	{
		size_t contentLengthPos = requestStr_.find("Content-Length");
		if (contentLengthPos == std::string::npos)
		{
			contentLengthPos = requestStr_.find("content-length");
		}
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
	else if (ft::caseInsensitiveFind(requestStr_, "Transfer-Encoding"))
	{
		// TODO: check if header value is chunked in a better way

		// Search for transfer encoding and check if value is chunked
		size_t transferEncodingPos = requestStr_.find("Transfer-Encoding");
		if (transferEncodingPos == std::string::npos)
		{
			transferEncodingPos = requestStr_.find("transfer-encoding");
		}
		if (transferEncodingPos != std::string::npos)
		{
			if (requestStr_.find("Chunked", transferEncodingPos)
					!= std::string::npos
				|| requestStr_.find("chunked", transferEncodingPos)
					   != std::string::npos) // Has Transfer-Encoding and
											 // presumed chunked
			{
				isChunked_ = true;
				return false;
			}
		}

		// Handle chunked transfer encoding
		// TODO: this is wrong!
		size_t chunkEndPos = requestStr_.find("0\r\n\r\n", bodyStartPos);
		if (chunkEndPos != std::string::npos)
		{
			extractExtraChars_(chunkEndPos + 5);
			return true;
		}
	}
	return false;
}

/**
 * @brief Extracts extra characters from the request string.
 *
 * Removes extra characters from the request string and adds them back to
 * the beginning of the client buffer for the next read.
 *
 * @param pos The position in the request string where the extra characters
 * start.
 */
void Client::extractExtraChars_(size_t pos)
{
	std::string extraChars = requestStr_.substr(pos);
	requestStr_ = requestStr_.substr(0, pos);
	clientBuffer_.str(extraChars + clientBuffer_.str());
	clientBuffer_.seekg(0, std::ios::beg); // Reset the get pointer
}

/**
 * @brief Checks for the presence of size-indicating headers.
 *
 * Determines if the request contains headers that indicate the presence
 * of a body, such as Content-Length or Transfer-Encoding.
 *
 * @return true if size-indicating headers are present, false otherwise.
 */
bool Client::hasSizeIndicator_(void)
{
	return ft::caseInsensitiveFind(requestStr_, "Content-Length")
		   || ft::caseInsensitiveFind(requestStr_, "Transfer-Encoding");
}

/**
 * @brief Resets the state of the Client object. Necessary after response has
 * been sent to client.
 */
void Client::reset_()
{
	requestStr_.clear();
	clientBuffer_.str("");
	clientBuffer_.clear();
	hasCompleteRequest_ = false;
	isChunked_ = false;
}

std::ostream &operator<<(std::ostream &os, const Client &rhs)
{
	os << "Client:" << std::endl;
	os << "File descriptor: " << rhs.getFd() << std::endl;
	os << "Is closed: " << rhs.isClosed() << std::endl;
	os << "Is chunked: " << rhs.isChunked() << std::endl;

	return os;
}
