#include "test.hpp"

Test(ServerException, SimpleMessage)
{
	ServerException ex("Test message");
	cr_assert(eq(str, ex.what(), "Test message"));
}

Test(ServerException, ArgumentReplacement)
{
	ServerException ex("Error with argument: %", 0, "argValue");
	cr_assert(eq(str, ex.what(), "Error with argument: argValue"));
}

Test(ServerException, ErrnoMessage)
{
	int				errNum = EINVAL;
	ServerException ex("Test message", errNum);
	std::string		expectedMessage =
		"Test message : (" + ft::toString(errNum) + ") " + strerror(errNum);
	cr_assert(eq(str, ex.what(), expectedMessage.c_str()));
}

Test(ServerException, ArgumentAndErrno)
{
	int				errNum = EINVAL;
	ServerException ex("Error: % fatal", errNum, "argValue");
	std::string		expectedMessage = "Error: argValue fatal : (" +
								  ft::toString(errNum) + ") " +
								  strerror(errNum);
	cr_assert(eq(str, ex.what(), expectedMessage.c_str()));
}
