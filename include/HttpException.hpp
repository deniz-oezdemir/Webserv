#include <exception>
#include <string>

class HttpException : public std::exception
{
  public:
	HttpException(void);
	HttpException(int code, const std::string &message);
	HttpException(const HttpException &other);
	HttpException &operator=(const HttpException &other);
	virtual ~HttpException(void) throw();

	int getCode(void) const;

	const char *what(void) const throw();

  private:
	int			_code;
	std::string _message;
};

