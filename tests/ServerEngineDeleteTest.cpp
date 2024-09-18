#include "HttpRequest.hpp"
#include "Logger.hpp"
#include "RequestParser.hpp"
#include "ServerConfig.hpp"
#include "ServerEngine.hpp"
#include <criterion/criterion.h>
#include <fstream>
#include <sstream>

std::string readFile(const std::string &filePath)
{
	std::ifstream file(filePath);
	if (!file.is_open())
	{
		std::cerr << "Error: Could not open file: " << file.is_open() << " "
				  << filePath << std::endl;
		return "";
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	if (buffer.str().empty())
	{
		std::cerr << "Error: File " << filePath
				  << " is empty or could not be read" << std::endl;
	}
	return buffer.str();
}

// Test for handling DELETE request
Test(ServerEngine, handleDeleteRequest)
{
	// Create a request string for DELETE method
	std::string requestStr = readFile("deleteRequest.txt");

	// Parse the request string into an HttpRequest object
	HttpRequest request = RequestParser::parseRequest(requestStr);

	ServerConfig config("test.config");
	config.parseFile(false, false);
	// config.setRootToAllServers("../www/website");

	{
		// Create a ServerEngine object
		ServerEngine serverEngine(config.getAllServersConfig());
		// serverEngine.start(); // This line is commented out

		// Call the handleDeleteRequest method
		std::string response = serverEngine.createResponse(request);

		std::cout << "\n\nTest Response:\n" << response << std::endl;
		// Assert the expected response
		cr_assert(
			response.find("200 OK") != std::string::npos,
			"Expected 200 OK response"
		);
		cr_assert(
			response.find("<!DOCTYPE html>") != std::string::npos,
			"Expected HTML content in response"
		);
	}
}
