#pragma once

#include <cstddef>
#include <sstream>
#include <string>

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
	bool hasSizeIndicator(void);

	int				 *fd_;
	std::stringstream clientBuffer_;
	std::string		  requestStr_;
	size_t			  bytesRead_;
	bool			  isChunked_;
	bool			  hasCompleteRequest_;
	bool			  isClosed_;
};

std::ostream &operator<<(std::ostream &os, const Client &rhs);
