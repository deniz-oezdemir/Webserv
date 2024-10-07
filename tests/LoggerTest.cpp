#include "test.hpp"

Test(Logger, setAndGetLevel)
{
	Logger::setLevel(Logger::DEBUG);
	cr_assert(Logger::getLevel() == Logger::DEBUG);

	Logger::setLevel(Logger::INFO);
	cr_assert(Logger::getLevel() == Logger::INFO);
}

Test(Logger, logToStdout)
{
	std::ostringstream capturedOutput;
	std::streambuf	  *originalCoutBuffer = std::cout.rdbuf();
	std::cout.rdbuf(capturedOutput.rdbuf());

	Logger::setLevel(Logger::DEBUG);
	Logger::log(Logger::DEBUG) << "This is a test log" << std::endl;

	cr_assert(
		capturedOutput.str().find("This is a test log") != std::string::npos
	);
	// Restore the original buffer
	std::cout.rdbuf(originalCoutBuffer);

	// Print the full message
	// Logger::log(Logger::DEBUG) << "This is a test log" << std::endl;
}

// Test(Logger, logToStderr)
// {
// 	std::ostringstream capturedOutput;
// 	std::streambuf *originalCerrBuffer = std::cerr.rdbuf();
// 	std::cerr.rdbuf(capturedOutput.rdbuf());
//
// 	Logger::log(Logger::ERROR) << "This is a test error" << std::endl;
// 	// Restore the original buffer
// 	std::cerr.rdbuf(originalCerrBuffer);
//
// 	cr_assert(capturedOutput.str().find("This is a test error") != std::string::npos);
// 	// Print the full message
// 	// Logger::log(Logger::ERROR) << "This is a test error" << std::endl;
// }

Test(Logger, loggerWithDebugLevel)
{
	std::ostringstream capturedOutput;
	std::streambuf	  *originalCoutBuffer = std::cout.rdbuf();
	std::cout.rdbuf(capturedOutput.rdbuf());

	Logger::setLevel(Logger::DEBUG);

  Logger::log(Logger::DEBUG) << "This is a test with debug level" << std::endl;
	cr_assert(
		capturedOutput.str().find("This is a test with debug level") != std::string::npos
	);
  Logger::log(Logger::INFO) << "This is a test with info level" << std::endl;
	cr_assert(
		capturedOutput.str().find("This is a test with info level") != std::string::npos
	);
  Logger::log(Logger::ERROR) << "This is a test with error level" << std::endl;
	cr_assert(
		capturedOutput.str().find("This is a test with error level") != std::string::npos
	);
	// Restore the original buffer
	std::cout.rdbuf(originalCoutBuffer);
	// Print all message =>
	// Logger::log(Logger::DEBUG) << "This is a test with error debug" << std::endl;
	// Logger::log(Logger::INFO) << "This is a test with info level" << std::endl;
	// Logger::log(Logger::WARN) << "This is a test with warn level" << std::endl;
	// Logger::log(Logger::ERROR) << "This is a test with error level" << std::endl;
}

Test(Logger, loggerWithInfoLevel)
{
	std::ostringstream capturedOutput;
	std::streambuf	  *originalCoutBuffer = std::cout.rdbuf();
	std::cout.rdbuf(capturedOutput.rdbuf());

	Logger::setLevel(Logger::INFO);

  Logger::log(Logger::DEBUG) << "This is a test with debug level" << std::endl;
	cr_assert(
		capturedOutput.str().find("This is a test with debug level") == std::string::npos
	);
  Logger::log(Logger::INFO) << "This is a test with info level" << std::endl;
	cr_assert(
		capturedOutput.str().find("This is a test with info level") != std::string::npos
	);
  Logger::log(Logger::ERROR) << "This is a test with error level" << std::endl;
	cr_assert(
		capturedOutput.str().find("This is a test with error level") != std::string::npos
	);
	// Restore the original buffer
	std::cout.rdbuf(originalCoutBuffer);
	// Print all message =>
	// Logger::log(Logger::DEBUG) << "This is a test with error debug" << std::endl;
	// Logger::log(Logger::INFO) << "This is a test with info level" << std::endl;
	// Logger::log(Logger::WARN) << "This is a test with warn level" << std::endl;
	// Logger::log(Logger::ERROR) << "This is a test with error level" << std::endl;
}

Test(Logger, loggerWithErrorLevel)
{
	std::ostringstream capturedOutput;
	std::streambuf	  *originalCoutBuffer = std::cout.rdbuf();
	std::cout.rdbuf(capturedOutput.rdbuf());

	Logger::setLevel(Logger::ERROR);

  Logger::log(Logger::DEBUG) << "This is a test with debug level" << std::endl;
	cr_assert(
		capturedOutput.str().find("This is a test with debug level") == std::string::npos
	);
  Logger::log(Logger::INFO) << "This is a test with info level" << std::endl;
	cr_assert(
		capturedOutput.str().find("This is a test with info level") == std::string::npos
	);
  Logger::log(Logger::ERROR) << "This is a test with error level" << std::endl;
	cr_assert(
		capturedOutput.str().find("This is a test with error level") != std::string::npos
	);
	// Restore the original buffer
	std::cout.rdbuf(originalCoutBuffer);
	// Print all message =>
	// Logger::log(Logger::DEBUG) << "This is a test with error debug" << std::endl;
	// Logger::log(Logger::INFO) << "This is a test with info level" << std::endl;
	// Logger::log(Logger::WARN) << "This is a test with warn level" << std::endl;
	// Logger::log(Logger::ERROR) << "This is a test with error level" << std::endl;
}
