#include "Client.hpp"
#include "HttpException.hpp"
#include "Logger.hpp"
#include "macros.hpp"
#include "utils.hpp"
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <exception>
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
	pollFd_ = rhs.pollFd_;
	requestStr_ = rhs.requestStr_;
	isChunked_ = rhs.isChunked_;
	hasCompleteRequest_ = rhs.hasCompleteRequest_;
	isClosed_ = rhs.isClosed_;
	isError_ = rhs.isError_;

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
	if (isClosed_ == true)
	{
		Logger::log(Logger::ERROR) << "Attempted to read from closed client."
								   << "Client info: " << *this << std::endl;
		throw std::invalid_argument("Cannot read from closed client.");
		return false;
	}
	if (isError_ == true)
	{
		Logger::log(Logger::ERROR)
			<< "Attempted to read from client with error."
			<< "Client info: " << *this << std::endl;
		throw std::invalid_argument("Cannot read from client with error.");
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

	std::vector<char> buffer(READ_BUFFER_SIZE);
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
	std::string tmp;
	if (isError_ == false)
	{
		if (hasCompleteRequest_ == true)
		{
			tmp = requestStr_;
			reset_();
		}
	}
	else
	{
		Logger::log(Logger::INFO, true)
			<< "Attempted to extract request from client with error." << *this
			<< std::endl;
		throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
	}

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

/**
 * @brief Reads data from the client buffer and processes it.
 *
 * Reads lines from the client buffer into the request string and checks if
 * a complete request has been received.
 */
void Client::readClientBuffer_(void)
{
	int emptyLineCount = 0;

	while (clientBuffer_.str().empty() == false && isClosed_ == false)
	{
		std::string line;
		std::getline(clientBuffer_, line, '\n');
		requestStr_ += line + "\n";

		if (line.empty())
		{
			emptyLineCount++;
		}
		else
		{
			emptyLineCount = 0;
		}

		// TODO: check this bullshit  manu
		if (emptyLineCount >= 3)
		{
			Logger::log(Logger::INFO)
				<< "Client sent three consecutive empty lines."
				<< "\nrequestStr_.length(): " << requestStr_.length()
				<< "\nrequestStr_: " << requestStr_ << std::endl;
			isClosed_ = true;
			isError_ = true;
			throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
			return;
		}

		if (requestStr_.length() > MAX_READ_SIZE)
		{
			Logger::log(Logger::INFO)
				<< "Client sent request over default buffer size limit."
				<< "\nrequestStr_.length(): " << requestStr_.length()
				<< "\nrequestStr_: " << requestStr_ << std::endl;
			isClosed_ = true;
			isError_ = true;
			throw HttpException(HTTP_400_CODE, HTTP_400_REASON);
			return;
		}

		if (isCompleteRequest_() == true)
		{
			hasCompleteRequest_ = true;
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
			moveExtraCharsToBuffer__(headerEndPos + 4);
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
		return handleContentLength_(bodyStartPos);
	}
	else if (ft::caseInsensitiveFind(requestStr_, "Transfer-Encoding"))
	{
		Logger::log(Logger::INFO)
			<< "Found chunked request: " << requestStr_ << std::endl;
		return handleChunkedEncoding_(bodyStartPos);
	}
	return false;
}

/**
 * @brief Handles the Content-Length header to determine if the body is fully
 * received.
 *
 * Checks the Content-Length header to determine if the entire body of the
 * request has been received. If the body is fully received, it moves any extra
 * characters to the buffer.
 *
 * @param bodyStartPos The position in the request string where the body starts.
 * @return true if the body is fully received, false otherwise.
 */
bool Client::handleContentLength_(size_t bodyStartPos)
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
		moveExtraCharsToBuffer__(bodyStartPos + contentLength);
		return true;
	}
	return false;
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
bool Client::processChunks_(size_t chunkStart)
{
	size_t		totalSize = 0;
	std::string concatenatedBody;

	while (true)
	{
		Logger::log(Logger::DEBUG)
			<< "Processing chunk starting at position: " << chunkStart
			<< std::endl;

		size_t chunkSizeEndPos = requestStr_.find("\r\n", chunkStart);
		if (chunkSizeEndPos == std::string::npos)
		{
			Logger::log(Logger::DEBUG)
				<< "Chunk size end position not found." << std::endl;
			return false;
		}

		std::string chunkSizeStr
			= requestStr_.substr(chunkStart, chunkSizeEndPos - chunkStart);
		size_t chunkSize = std::strtoul(chunkSizeStr.c_str(), NULL, 16);
		Logger::log(Logger::DEBUG)
			<< "Chunk size: " << chunkSize << " (hex: " << chunkSizeStr << ")"
			<< std::endl;

		if (chunkSize == 0)
		{
			size_t endOfChunksPos = chunkSizeEndPos + 2;
			if (requestStr_.find("\r\n", endOfChunksPos) == endOfChunksPos)
			{
				moveExtraCharsToBuffer__(endOfChunksPos + 2);
				hasCompleteRequest_ = true;
				isChunked_ = false;

				// Extract headers
				size_t		headerEndPos = requestStr_.find("\r\n\r\n");
				std::string headers = requestStr_.substr(0, headerEndPos + 4);

				// Construct the final request string with headers and
				// concatenated body
				std::string resultBody = headers + concatenatedBody;
				requestStr_ = resultBody;

				Logger::log(Logger::INFO)
					<< "End of chunks reached. Complete request: "
					<< requestStr_ << std::endl;
				return true;
			}
			Logger::log(Logger::DEBUG)
				<< "End of chunks not properly terminated." << std::endl;
			return false;
		}

		size_t chunkDataStartPos = chunkSizeEndPos + 2;
		size_t chunkDataEndPos = chunkDataStartPos + chunkSize;
		if (requestStr_.size() < chunkDataEndPos + 2)
		{
			Logger::log(Logger::DEBUG)
				<< "Incomplete chunk data. Expected end position: "
				<< chunkDataEndPos + 2 << std::endl;
			return false;
		}

		// Append the chunk data to the concatenated body
		std::string chunkData
			= requestStr_.substr(chunkDataStartPos, chunkSize);
		concatenatedBody += chunkData;
		totalSize += chunkSize;

		Logger::log(Logger::DEBUG) << "Chunk data: " << chunkData << std::endl;
		Logger::log(Logger::DEBUG)
			<< "Total size so far: " << totalSize << std::endl;

		Logger::log(Logger::DEBUG)
			<< "Chunk data processed. Next chunk starts at: "
			<< chunkDataEndPos + 2 << std::endl;
		chunkStart = chunkDataEndPos + 2;
	}
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
void Client::moveExtraCharsToBuffer__(size_t pos)
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
	os << "Is error: " << rhs.isError() << std::endl;
	os << "Is chunked: " << rhs.isChunked() << std::endl;

	return os;
}
