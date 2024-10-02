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
	int	 getFd(void) const;

  private:
	Client(void);

	void readClientBuffer_(void);
	bool isCompleteRequest_(void);
	bool hasSizeIndicator_(void);
	bool isBodyFullyReceived_(size_t headerEndPos);
	void moveExtraCharsToBuffer__(size_t pos);
	void reset_();
	bool handleContentLength_(size_t bodyStartPos);
	bool handleChunkedEncoding_(size_t bodyStartPos);
	bool processChunks_(size_t chunkStart);

	int				  pollFd_;
	std::stringstream clientBuffer_;
	std::string		  requestStr_;
	bool			  isChunked_;
	bool			  hasCompleteRequest_;
	bool			  isClosed_;
	bool			  isError_;
};

std::ostream &operator<<(std::ostream &os, const Client &rhs);
