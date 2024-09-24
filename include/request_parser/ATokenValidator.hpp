#pragma once

#include <map>
#include <string>
#include <vector>
class ATokenValidator
{
  public:
	// clang-format off
	static void validateTokens(
		std::map<std::string, std::vector<std::string> > &headers
	);
	// clang-format on

  private:
	ATokenValidator(void);
	ATokenValidator(const ATokenValidator &src);
	~ATokenValidator(void);
	ATokenValidator &operator=(const ATokenValidator &rhs);
};
