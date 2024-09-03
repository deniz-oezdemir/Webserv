#include "./test.hpp"

Test(ServerInput, inputWithOnlyPath)
{
	char	**argv = new char*[2];
	int		argc = 2;
	argv[0] = (char*)"./server";
	argv[1] = (char*)"test.config";
	ServerInput serverInput(argc, argv);
	delete[] argv;

	cr_assert(eq(int, serverInput.hasThisFlag(ServerInput::HELP), false));
	cr_assert(eq(int, serverInput.hasThisFlag(ServerInput::V_LITE), false));
	cr_assert(eq(int, serverInput.hasThisFlag(ServerInput::V_FULL), false));
	cr_assert(eq(int, serverInput.hasThisFlag(ServerInput::TEST), false));
	cr_assert(eq(int, serverInput.hasThisFlag(ServerInput::TEST_PRINT), false));
	cr_assert(eq(str, serverInput.getFilePath(), "test.config"));
}

Test(ServerInput, inputWithAllFlags)
{
	char	**argv = new char*[6];
	int		argc = 6;
	argv[0] = (char*)"./server";
	argv[1] = (char*)"-t";
	argv[2] = (char*)"-T";
	argv[3] = (char*)"-v";
	argv[4] = (char*)"-V";
	argv[5] = (char*)"-h";
	ServerInput serverInput(argc, argv);
	delete[] argv;

	cr_assert(eq(int, serverInput.hasThisFlag(ServerInput::HELP), true));
	cr_assert(eq(int, serverInput.hasThisFlag(ServerInput::V_LITE), true));
	cr_assert(eq(int, serverInput.hasThisFlag(ServerInput::V_FULL), true));
	cr_assert(eq(int, serverInput.hasThisFlag(ServerInput::TEST), true));
	cr_assert(eq(int, serverInput.hasThisFlag(ServerInput::TEST_PRINT), true));
	cr_assert(eq(str, serverInput.getFilePath(), "./default.config"));
}
