#pragma once

#include <cstddef>
#include <sstream>
#include <string>

/**
 * @class Client
 * @brief Handles client connections and processes incoming requests.
 *
 * The Client class is responsible for managing client connections in a server
 * program. It reads data from a file descriptor, processes incoming requests,
 * and maintains the state of the connection. The class provides methods to
 * check if a complete request has been received, extract the request string,
 * and determine if the connection is closed.
 */
class Client
{
  public:
	Client(int *fd);
	~Client(void);

	bool hasRequest(void);

	std::string extractRequestStr(void);

	// Getters
	bool hasCompleteRequest(void) const;
	bool isClosed(void) const;
	int *getFd(void) const;

  private:
	Client(void);
	Client(const Client &src);
	Client &operator=(const Client &rhs);

	void readClientBuffer_(void);
	bool isCompleteRequest_(void);
	bool hasSizeIndicator_(void);
	bool isBodyFullyReceived_(size_t headerEndPos);
	void extractExtraChars_(size_t pos);

	int				 *fd_;
	std::stringstream clientBuffer_;
	std::string		  requestStr_;
	bool			  isChunked_;
	bool			  hasCompleteRequest_;
	bool			  isClosed_;
};

std::ostream &operator<<(std::ostream &os, const Client &rhs);
