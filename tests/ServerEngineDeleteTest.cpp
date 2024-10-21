#include "test.hpp"
#include <criterion/criterion.h>
#include <fstream>
#include <sstream>

// Test for handling DELETE request
Test(ServerEngine, handleDeleteRequest_)
{
	// Create a request string for DELETE method
	std::string requestStr = ft::readFile("./test_requests/deleteRequest.txt");

	// Parse the request string into an HttpRequest object
	HttpRequest request = RequestParser::parseRequest(requestStr);

	ServerConfig config("test.config");
	config.parseFile(false, false);
	// config.setRootToAllServers("../www/website");

	{
		// Create a ServerEngine object
		ServerEngine serverEngine(config.getAllServersConfig());
		// serverEngine.start(); // This line is commented out

		// Call the handleDeleteRequest_ method
		std::string response = serverEngine.createResponse(request);

		std::cout << "\n\nTest Response:\n" << response << std::endl;
		// Assert the expected response
		cr_assert(
			response.find("200 OK") != std::string::npos,
			"Expected 200 OK response"
		);
		cr_assert(
			response.find("File Deleted Successfull") != std::string::npos,
			"Expected File Deleted Successfull"
		);
	}
}
