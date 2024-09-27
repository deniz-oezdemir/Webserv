#include "test.hpp"
#include <criterion/criterion.h>
#include <fstream>
#include <sstream>

// Test for handling POST request
Test(ServerEngine, handlePostRequest_)
{
	// Create a request string for POST method
	std::string requestStr = ft::readFile("postRequest.txt");

	// Parse the request string into an HttpRequest object
	HttpRequest request = RequestParser::parseRequest(requestStr);

	ServerConfig config("test.config");
	config.parseFile(false, false);
	// config.setRootToAllServers("../www/website");

	{
		// Create a ServerEngine object
		ServerEngine serverEngine(config.getAllServersConfig());
		// serverEngine.start(); // This line is commented out

		// Call the handlePostRequest_ method
		std::string response = serverEngine.createResponse(request);

		std::cout << "\n\nTest Response:\n" << response << std::endl;
		// Assert the expected response
		cr_assert(
			response.find("200 OK") != std::string::npos,
			"Expected 200 OK response"
		);
		cr_assert(
			response.find("File Uploaded Successfully") != std::string::npos,
			"Expected File Uploaded Successfully"
		);
	}
}
