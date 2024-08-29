#include "test.hpp"

Test(ServerConfig, CorrectPathArgument)
{
	try
	{
		ServerConfig config("../default.config");
		cr_assert(eq(int, config.getFile().is_open(), true));
	}
	catch (std::exception& e)
	{
		cr_fail();
	}
}

Test(ServerConfig, WrongPathArgument)
{
	try
	{
		ServerConfig config("../fake.config");
		cr_assert(eq(int, config.getFile().is_open(), false));
	}
	catch (std::exception& e)
	{
		cr_assert(
			eq(str,
			   e.what(),
			   "Could not open the file [../fake.config] : (2) No such file or "
			   "directory")
		);
	}
}
