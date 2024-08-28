#include "./test.hpp"

Test(ServerInput, initServerWithOnlyThePath)
{
	char **argv = new char*[2];
	argv[0] = (char*)"./server";
	argv[1] = (char*)"test.config";
	ServerInput serverInput(2, argv);

	cr_assert(eq(int, serverInput.hasThisFlag(ServerInput::HELP), false));
	cr_assert(eq(int, serverInput.hasThisFlag(ServerInput::V_LITE), false));
	cr_assert(eq(int, serverInput.hasThisFlag(ServerInput::V_FULL), false));
	cr_assert(eq(int, serverInput.hasThisFlag(ServerInput::TEST), false));
	cr_assert(eq(int, serverInput.hasThisFlag(ServerInput::TEST_PRINT), false));
	cr_assert(eq(str, serverInput.getFilePath(), "test.config"));
}
