#pragma once

#include <map>
#include <string>
#include <vector>
class TokenValidator
{
  public:
	// clang-format off
	static void validateTokens(
		std::map<std::string, std::vector<std::string> > &headers
	);
	// clang-format on

  private:
	TokenValidator(void);
	TokenValidator(const TokenValidator &src);
	~TokenValidator(void);
	TokenValidator &operator=(const TokenValidator &rhs);
};
