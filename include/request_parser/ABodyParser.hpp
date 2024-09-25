#pragma once

#include <istream>
#include <map>
#include <vector>

class ABodyParser
{
  public:
	static void parseBody(
		std::istream	  &requestStream,
		const std::string &method,
		// clang-format off
	const std::map<std::string, std::vector<std::string> > &headers,
		// clang-format on
		std::vector<char> *body
	);

  private:
	ABodyParser(void);
	ABodyParser(const ABodyParser &src);
	~ABodyParser(void);
	ABodyParser &operator=(const ABodyParser &rhs);

	static void checkBody_(
		const std::string &method,
		// clang-format off
		const std::map<std::string, std::vector<std::string> > &headers,
		// clang-format on
		const std::vector<char> &body
	);
};
