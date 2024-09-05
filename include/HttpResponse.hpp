#pragma once

#include <map>
#include <string>

class HttpResponse
{
  public:
	HttpResponse();
	~HttpResponse();

	void setStatusCode(int code);
	void setReasonPhrase(const std::string &phrase);
	void setHeader(const std::string &key, const std::string &value);
	void setBody(const std::string &body);

	std::string toString() const;

  private:
	int								   statusCode_;
	std::string						   reasonPhrase_;
	std::map<std::string, std::string> headers_;
	std::string						   body_;
};
