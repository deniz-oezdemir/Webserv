#pragma once

#include <istream>
#include <map>
#include <vector>
class BodyParser
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
	BodyParser(void);
	BodyParser(const BodyParser &src);
	~BodyParser(void);
	BodyParser &operator=(const BodyParser &rhs);

	static void checkBody_(
		const std::string &method,
		// clang-format off
		const std::map<std::string, std::vector<std::string> > &headers,
		// clang-format on
		const std::vector<char> &body
	);
};
