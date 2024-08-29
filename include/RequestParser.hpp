#pragma once
#include "HttpRequest.hpp"
#include <cstddef>
#include <iostream>

class RequestParser
{
  public:
	static HttpRequest parseRequest(std::string str);

  private:
	RequestParser(void);
	~RequestParser(void);
	RequestParser(const RequestParser &src);
	RequestParser &operator=(const RequestParser &rhs);
};
