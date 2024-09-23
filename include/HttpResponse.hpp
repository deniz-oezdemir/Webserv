#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

class HttpResponse
{
  public:
	HttpResponse();
	~HttpResponse();

	void setStatusCode(int code);
	void setReasonPhrase(const std::string &phrase);
	void setHeader(const std::string &key, const std::string &value);
	void setBody(const std::string &body);

	std::string const &getHeader(std::string const &key) const;

	std::string toString() const;

  private:
	int			statusCode_;
	std::string reasonPhrase_;
	// vector of pairs as unordered map only introduced with C++11
	std::vector<std::pair<std::string, std::string> > headers_;
	std::string										 body_;
};
