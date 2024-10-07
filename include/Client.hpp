#pragma once

#include <cstddef>
#include <sstream>
#include <string>

class Client
{
  public:
	Client(int pollFd);
	~Client(void);
	Client(const Client &src);
	Client &operator=(const Client &rhs);

	/**
	 * @brief Checks if there is a complete request from the client.
	 *
	 * Reads data from the file descriptor into the client buffer and processes
	 * it to determine if a complete request has been received.
	 *
	 * @return true if a complete request has been received, false otherwise.
	 */
	bool		hasRequestReady(void);
	std::string extractRequestStr(void);

	// Getters
	bool isClosed(void) const;
	bool isError(void) const;
	bool isChunked(void) const;
	bool areHeadersRead(void) const;
	int	 getFd(void) const;

	void setIsClosed(bool closed);

  private:
	Client(void);

	void   readClientBuffer_(void);
	bool   isCompleteRequest_(void);
	bool   hasSizeIndicator_(void);
	void   moveExtraCharsToBuffer__(size_t pos);
	void   reset_(void);
	bool   handleChunkedEncoding_(size_t bodyStartPos);
	bool   processChunks_(size_t chunkStart);
	bool   areHeadersComplete_(void);
	size_t getBodySize_(void);

	int				  pollFd_;
	std::stringstream clientBuffer_;
	std::string		  requestStr_;
	size_t			  totalBytesReadFromFd_;
	bool			  isChunked_;
	bool			  hasCompleteRequest_;
	bool			  isClosed_;
	bool			  isError_;
	bool			  areHeadersRead_;
	bool			  readingPartialBody_;
	size_t			  nextReadSize_;
};

std::ostream &operator<<(std::ostream &os, const Client &rhs);
