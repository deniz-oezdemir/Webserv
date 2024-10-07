#include "Client.hpp"
#include "HttpException.hpp"
#include "Logger.hpp"
#include "macros.hpp"
#include "utils.hpp"
#include <cerrno>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <vector>

Client::Client(int pollFd) : pollFd_(pollFd)
{
	hasCompleteRequest_ = false;
	isChunked_ = false;
	isClosed_ = false;
	isError_ = false;
	areHeadersRead_ = false;
	nextReadSize_ = 1;
	totalBytesReadFromFd_ = 0;
	readingPartialBody_ = false;
}

Client::~Client(void)
{
	reset_();
}

Client::Client(const Client &src)
{
	*this = src;
}

Client &Client::operator=(const Client &rhs)
{
	Logger::log(Logger::DEBUG)
		<< "Client copy operator init. rhs:" << rhs << std::endl;

	pollFd_ = rhs.pollFd_;
	requestStr_ = rhs.requestStr_;
	isChunked_ = rhs.isChunked_;
	hasCompleteRequest_ = rhs.hasCompleteRequest_;
	isClosed_ = rhs.isClosed_;
	isError_ = rhs.isError_;
	areHeadersRead_ = rhs.areHeadersRead_;
	nextReadSize_ = rhs.nextReadSize_;
	totalBytesReadFromFd_ = rhs.totalBytesReadFromFd_;
	readingPartialBody_ = rhs.readingPartialBody_;

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
	Logger::log(Logger::DEBUG) << "hasRequestReady init." << std::endl;

	if (isClosed_ == true)
	{
		Logger::log(Logger::ERROR)
			<< "hasRequestReady: Attempted to read from closed client."
			<< "Client info: " << *this << std::endl;
		throw std::invalid_argument("Cannot read from closed client.");
		return false;
	}
	if (isError_ == true)
	{
		Logger::log(Logger::ERROR)
			<< "hasRequestReady: Attempted to read from client with error."
			<< "Client info: " << *this << std::endl;
		throw std::invalid_argument("Cannot read from client with error.");
		return false;
	}

	if (hasCompleteRequest_)
	{
		Logger::log(Logger::ERROR)
			<< "hasRequestReady: Attemped to read from client that has "
			   "full request pending extraction! Client info: "
			<< *this << std::endl;
		return true;
	}

	if (totalBytesReadFromFd_ + nextReadSize_ > MAX_REQUEST_SIZE)
	{
		std::string debugLine;
		std::getline(clientBuffer_, debugLine);
		Logger::log(Logger::INFO)
			<< "hasRequestReady: Client sent request over default buffer size "
			   "limit."
			<< std::endl
			<< "totalBytesReadFromFd_: " << totalBytesReadFromFd_ << std::endl
			<< "nextReadSize_: " << nextReadSize_ << std::endl;
		isClosed_ = true;
		isError_ = true;
		throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
		return false;
	}

	std::vector<char> buffer(nextReadSize_);
	long int bytesReadFromFd = read(pollFd_, buffer.data(), buffer.size());
	totalBytesReadFromFd_ += bytesReadFromFd;

	if (bytesReadFromFd < 0)
	{
		Logger::log(Logger::ERROR)
			<< "hasRequestReady:Failed to read from client: ("
			<< ft::toString(errno) << ") " << strerror(errno) << std::endl;
		isClosed_ = true;
		isError_ = true;
		return false;
	}
	else if (readingPartialBody_ == false && bytesReadFromFd == 0)
	{
		Logger::log(Logger::INFO)
			<< "hasRequestReady: Client disconnected: " << *this << std::endl;
		isClosed_ = true;
		hasCompleteRequest_ = true;
		return false;
	}

	clientBuffer_.write(buffer.data(), bytesReadFromFd);
	requestStr_ = clientBuffer_.str();

	if (bytesReadFromFd != (long)nextReadSize_)
	{
		Logger::log(Logger::DEBUG)
			<< "hasRequestReady: bytesReadFromFd: " << bytesReadFromFd
			<< " do not match expected nextReadSize: " << nextReadSize_
			<< std::endl;
		nextReadSize_ = nextReadSize_ - bytesReadFromFd;
		readingPartialBody_ = true;
		return false;
	}

	if (readingPartialBody_ == true && bytesReadFromFd == (long)nextReadSize_)
	{
		Logger::log(Logger::DEBUG)
			<< "hasRequestReady: Body arrived incomplete and has now been "
			   "fully read."
			<< std::endl;
		hasCompleteRequest_ = true;
		return true;
	}

	if (requestStr_.length() != totalBytesReadFromFd_)
	{
		Logger::log(Logger::ERROR)
			<< "hasRequestReady: Read error. requestStr_ length: "
			<< requestStr_.length()
			<< " do not match expected totalBytesReadFromFd_: "
			<< totalBytesReadFromFd_ << std::endl;
		isClosed_ = true;
		isError_ = true;
		return false;
	}

	if (areHeadersRead_ == true && nextReadSize_ != 1)
	{
		Logger::log(Logger::DEBUG)
			<< "hasRequestReady: Headers and body read." << std::endl;
		hasCompleteRequest_ = true;
		return true;
	}

	readClientBuffer_();
	return hasCompleteRequest_;
}

std::string Client::extractRequestStr(void)
{
	std::string tmp;
	if (isError_ == false)
	{
		if (hasCompleteRequest_ == true)
		{
			tmp = requestStr_;
			reset_();
			return tmp;
		}
		else
		{
			Logger::log(Logger::INFO, true)
				<< "Attempted to extract request from client not ready."
				<< *this << std::endl;
		}
	}
	else
	{
		Logger::log(Logger::INFO, true)
			<< "Attempted to extract request from client with error." << *this
			<< std::endl;
		throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
	}
	Logger::log(Logger::INFO, true)
		<< "extractRequestStr returning empty string." << *this << std::endl;

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

bool Client::isError(void) const
{
	return isError_;
}

bool Client::isChunked(void) const
{
	return isChunked_;
}

bool Client::areHeadersRead(void) const
{
	return areHeadersRead_;
}

/**
 * @brief Reads data from the client buffer and processes it.
 *
 * Reads lines from the client buffer into the request string and checks if
 * a complete request has been received. Handles both regular and chunked
 * transfer encoding.
 */
void Client::readClientBuffer_(void)
{
	areHeadersRead_ = areHeadersComplete_();

	if (areHeadersRead_ == true)
	{
		Logger::log(Logger::DEBUG)
			<< "readClientBuffer_:: Headers all read." << std::endl;
		if (hasSizeIndicator_() == false)
		{
			Logger::log(Logger::INFO)
				<< "readClientBuffer_: Client headers complete and no body."
				<< std::endl;
			hasCompleteRequest_ = true;
			return;
		}
		else
		{
			Logger::log(Logger::INFO
			) << "readClientBuffer_: Client headers complete and contains body."
			  << std::endl;

			if (isChunked_)
			{
				Logger::log(Logger::INFO)
					<< "readClientBuffer_: Client contains chunked encoding."
					<< std::endl;
				hasCompleteRequest_
					= processChunks_(requestStr_.find("\r\n\r\n") + 4);
			}
			else
			{
				nextReadSize_ = getBodySize_();
				Logger::log(Logger::INFO)
					<< "readClientBuffer_: Client headers read and "
					   "nextReadSize_ set to: "
					<< nextReadSize_ << std::endl;
			}
			return;
		}
	}
	else
	{
		Logger::log(Logger::DEBUG)
			<< "readClientBuffer_: Headers still not fully read. Read "
			<< requestStr_.length() << " bytes from FD." << std::endl;
		hasCompleteRequest_ = false;
		return;
	}
}

/**
 * @brief Checks if the headers are complete.
 *
 * Determines if the request string contains a complete set of HTTP headers
 * by checking for the end of headers.
 *
 * @return true if the headers are complete, false otherwise.
 */
bool Client::areHeadersComplete_(void)
{
	return requestStr_.find("\r\n\r\n") != std::string::npos;
}

/**
 * @brief Handles the Transfer-Encoding header to determine if the body is fully
 * received.
 *
 * Checks the Transfer-Encoding header to determine if the request uses chunked
 * transfer encoding. If it does, it processes the chunks to determine if the
 * entire body has been received.
 *
 * @param bodyStartPos The position in the request string where the body starts.
 * @return true if the body is fully received, false otherwise.
 */
bool Client::handleChunkedEncoding_(size_t bodyStartPos)
{
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
				   != std::string::npos)
		{
			isChunked_ = true;
			return processChunks_(bodyStartPos);
		}
	}
	else
	{
		Logger::log(Logger::INFO)
			<< "Parsing error with chunked! Fd set to close." << std::endl;
		isClosed_ = true;
	}
	return false;
}

/**
 * @brief Processes the chunks in a chunked transfer encoding request.
 *
 * Iterates through the chunks in the request to determine if the entire body
 * has been received. If the end of the chunks is reached, it moves any extra
 * characters to the buffer and marks the request as complete. The resulting
 * request will contain the original headers and the concatenated body without
 * size indications.
 *
 * @param chunkStart The position in the request string where the first chunk
 * starts.
 * @return true if the body is fully received, false otherwise.
 */
bool Client::processChunks_(size_t bodyStartPos)
{
	size_t pos = bodyStartPos;
	while (pos < requestStr_.size())
	{
		size_t chunkSizeEnd = requestStr_.find("\r\n", pos);
		if (chunkSizeEnd == std::string::npos)
		{
			return false; // Incomplete chunk size line
		}

		std::string chunkSizeStr = requestStr_.substr(pos, chunkSizeEnd - pos);
		size_t		chunkSize;
		std::istringstream(chunkSizeStr) >> std::hex >> chunkSize;
		pos = chunkSizeEnd + 2; // Move past the \r\n

		if (chunkSize == 0)
		{
			// Last chunk
			hasCompleteRequest_ = true;
			return true;
		}

		if (pos + chunkSize + 2 > requestStr_.size())
		{
			return false; // Incomplete chunk data
		}

		pos += chunkSize + 2; // Move past the chunk data and \r\n
	}

	return false;
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
 * @brief Gets the size of the body based on headers.
 *
 * Determines the size of the body based on the Content-Length or
 * Transfer-Encoding headers.
 *
 * @return The size of the body.
 */
size_t Client::getBodySize_(void)
{
	size_t bodySize = 0;
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
		bodySize = std::atoi(contentLengthStr.c_str());
	}
	else if (ft::caseInsensitiveFind(requestStr_, "Transfer-Encoding"))
	{
		// Handle chunked transfer encoding
		bodySize = std::string::npos; // Indicate chunked encoding
	}
	return bodySize;
}

/**
 * @brief Resets the state of the Client object. Necessary after response has
 * been sent to client.
 *
 * Clears the request string and client buffer, and resets state variables
 * to their initial values.
 */
void Client::reset_(void)
{
	requestStr_.clear();
	clientBuffer_.str("");
	clientBuffer_.clear();
	hasCompleteRequest_ = false;
	isChunked_ = false;
	areHeadersRead_ = false;
	nextReadSize_ = 1;
	totalBytesReadFromFd_ = 0;
	readingPartialBody_ = false;
}

std::ostream &operator<<(std::ostream &os, const Client &rhs)
{
	os << "Client:" << std::endl;
	os << "File descriptor: " << rhs.getFd() << std::endl;
	os << "Is closed: " << rhs.isClosed() << std::endl;
	os << "Is error: " << rhs.isError() << std::endl;
	os << "Is chunked: " << rhs.isChunked() << std::endl;
	os << "Headers fully read: " << rhs.areHeadersRead() << std::endl;

	return os;
}
