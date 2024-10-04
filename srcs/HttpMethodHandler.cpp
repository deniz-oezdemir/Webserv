#include "HttpMethodHandler.hpp"
#include "HttpErrorHandler.hpp"
#include "HttpResponse.hpp"
#include "Logger.hpp"
#include "macros.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

std::string HttpMethodHandler::handleRequest(
	HttpRequest const &request,
	Server const	  &server,
	std::string const &method
)
{
	if (method == "GET")
	{
		Logger::log(Logger::DEBUG)
			<< "Handling GET request: " << request.getUri() << std::endl;
		return handleGetRequest_(request, server);
	}
	else if (method == "POST")
	{
		Logger::log(Logger::DEBUG)
			<< "Handling POST request: " << request.getUri() << std::endl;
		return handlePostRequest_(request, server);
	}
	else if (method == "DELETE")
	{
		Logger::log(Logger::DEBUG)
			<< "Handling DELETE request: " << request.getUri() << std::endl;
		return handleDeleteRequest_(request, server);
	}
	return HttpErrorHandler::getErrorPage(501, true);
}

std::string HttpMethodHandler::handleGetRequest_(
	const HttpRequest &request,
	Server const	  &server
)
{
	std::string uri = request.getUri();
	bool		keepAlive = request.getKeepAlive();

	// clang-format off
	std::map<std::string, std::vector<std::string> > location; // clang-format on

	if (server.isThisLocation(uri))
		location = server.getThisLocation(uri);
	else
		return HttpErrorHandler::getErrorPage(404, keepAlive);

	// Check for redirections
	std::string redirection = handleRedirection_(location, keepAlive);
	if (!redirection.empty())
		return redirection;

	std::string filepath = getFilePath_(uri, location, server);
	std::string rootdir = getRootDir_(location, server);

	// Check for authorized methods
	if (!validateMethod_(location, "GET"))
		return handleErrorResponse_(server, 405, rootdir, keepAlive);

	// Check for max body size
	if (!validateBodySize_(location, request, server))
		return handleErrorResponse_(server, 413, rootdir, keepAlive);

	if (isCgiRequest_(location, uri))
		return handleCgiRequest_(
			filepath,
			getCgiInterpreter_(location),
			request,
			keepAlive,
			server,
			rootdir
		);
	// Check if the request is for a directory and handle autoindex
	if (isDirectory_(filepath))
	{
		// Search for index file in the directory
		filepath = findIndexFile_(filepath, location, server);
		if (filepath.empty())
		{
			if (isAutoIndexEnabled_(location))
				return handleAutoIndex_(rootdir, uri, server, keepAlive);
			else
				return handleErrorResponse_(server, 404, rootdir, keepAlive);
		}
	}

	// Check if the file exists and return the response
	return createFileGetResponse_(filepath, rootdir, server, keepAlive);
}

std::string HttpMethodHandler::handlePostRequest_(
	const HttpRequest &request,
	Server const	  &server
)
{
	std::string uri = request.getUri();
	bool		keepAlive = request.getKeepAlive();

	// clang-format off
	std::map<std::string, std::vector<std::string> > location; // clang-format on

	if (server.isThisLocation(uri))
		location = server.getThisLocation(uri);
	else
		return HttpErrorHandler::getErrorPage(404, keepAlive);

	std::string rootdir = getRootDir_(location, server);
	std::string uploadpath = getUploadPath_(location);
	if (uploadpath.empty())
		uploadpath = rootdir + uri;

	// Check for authorized methods
	if (!validateMethod_(location, "POST"))
		return handleErrorResponse_(server, 405, rootdir, keepAlive);
	// Check for max body size
	if (!validateBodySize_(location, request, server))
		return handleErrorResponse_(server, 413, rootdir, keepAlive);

	if (isCgiRequest_(location, uri))
		return handleCgiRequest_(
			rootdir + uri,
			getCgiInterpreter_(location),
			request,
			keepAlive,
			server,
			rootdir,
			uploadpath
		);

	return createFilePostResponse_(
		request, rootdir, location, uploadpath, server, keepAlive
	);
}

std::string HttpMethodHandler::handleDeleteRequest_(
	const HttpRequest &request,
	Server const	  &server
)
{
	std::string uri = request.getUri();
	bool		keepAlive = request.getKeepAlive();

	// clang-format off
	std::map<std::string, std::vector<std::string> > location; // clang-format on

	if (server.isThisLocation(uri))
		location = server.getThisLocation(uri);
	else
		return HttpErrorHandler::getErrorPage(404, keepAlive);

	std::string rootdir = getRootDir_(location, server);

	// Check for authorized methods
	if (!validateMethod_(location, "DELETE"))
		return handleErrorResponse_(server, 405, rootdir, keepAlive);
	// Check for max body size
	if (!validateBodySize_(location, request, server))
		return handleErrorResponse_(server, 413, rootdir, keepAlive);

	std::string filepath = getFilePath_(uri, location, server);
	if (isDirectory_(filepath))
		return handleErrorResponse_(server, 403, rootdir, keepAlive);

	if (isCgiRequest_(location, uri))
		return handleCgiRequest_(
			filepath,
			getCgiInterpreter_(location),
			request,
			keepAlive,
			server,
			rootdir
		);

	return createDeleteResponse_(
		filepath, rootdir, location, server, keepAlive
	);
}

std::string HttpMethodHandler::handleErrorResponse_(
	Server const	  &server,
	int const		  &errorCode,
	std::string const &rootdir,
	bool const		  &keepAlive
)
{
	std::string errorResponse
		= HttpErrorHandler::getErrorPage(server, errorCode, rootdir, keepAlive);
	if (!errorResponse.empty())
		return errorResponse;
	else
		return HttpErrorHandler::getErrorPage(errorCode, keepAlive);
}

// clang-format off
std::string HttpMethodHandler::handleRedirection_(
	std::map<std::string, std::vector<std::string> > const &location,
	bool const											  &keepAlive
)
{
	std::map<std::string, std::vector<std::string> >::const_iterator it
		= location.find("return"); // clang-format on
	if (it != location.end() && !it->second.empty())
		return handleReturnDirective_(it->second, keepAlive);
	return "";
}

std::string HttpMethodHandler::handleReturnDirective_(
	std::vector<std::string> const &returnDirective,
	bool const					   &keepAlive
)
{
	HttpResponse response;
	int			 statusCode = 301;

	if (returnDirective.size() == 2)
	{
		statusCode = std::atoi(returnDirective[0].c_str());
		response.setHeader("Location", returnDirective[1]);
	}
	else if (returnDirective.size() == 1)
	{
		response.setHeader("Location", returnDirective[0]);
	}

	Logger::log(Logger::DEBUG)
		<< "Handling return directive: [" << statusCode << "] "
		<< ft::getStatusCodeReason(statusCode)
		<< "To: " << response.getHeader("Location") << std::endl;

	response.setStatusCode(statusCode);
	response.setReasonPhrase(ft::getStatusCodeReason(statusCode));
	response.setHeader("Server", SERVER_NAME);
	response.setHeader("Date", ft::createTimestamp());
	response.setHeader("Content-Type", "text/html; charset=UTF-8");
	response.setHeader("Content-Length", "0");
	if (keepAlive)
		response.setHeader("Connection", "keep-alive");
	else
		response.setHeader("Connection", "close");

	return response.toString();
}

// clang-format off
bool HttpMethodHandler::validateMethod_(
	std::map<std::string, std::vector<std::string> > const &location,
	std::string const									  &method
)
{
	std::map<std::string, std::vector<std::string> >::const_iterator it
		= location.find("limit_except"); // clang-format on

	if (it != location.end() && !it->second.empty())
	{
		if (std::find(it->second.begin(), it->second.end(), method)
			== it->second.end())
			return false;
	}
	return true;
}

// clang-format off
bool HttpMethodHandler::validateBodySize_(
	std::map<std::string, std::vector<std::string> > const &location,
	HttpRequest	const								  &request,
	Server const									  &server
)
{
	std::map<std::string, std::vector<std::string> >::const_iterator it
		= location.find("client_body_size"); // clang-format on
	//
	if (it != location.end() && !it->second.empty())
	{
		if (request.getBody().size() > ft::stringToULong(it->second[0]))
			return false;
	}
	else if (request.getBody().size() > server.getClientMaxBodySize())
		return false;
	return true;
}

// clang-format off
std::string HttpMethodHandler::getFilePath_(
	std::string const									  &uri,
	std::map<std::string, std::vector<std::string> > const &location,
	Server const										  &server
) // clang-format on
{
	std::string rootdir = location.find("root") != location.end()
								  && !location.at("root").empty()
							  ? location.at("root")[0]
							  : server.getRoot();
	return rootdir + uri;
}

// clang-format off
std::string HttpMethodHandler::getUploadPath_(
	std::map<std::string, std::vector<std::string> > const &location
) // clcng-format on
{
	return location.find("upload_store") != location.end()
				   && !location.at("upload_store").empty()
			   ? location.at("upload_store")[0]
			   : "";
}

// clang-format off
std::string HttpMethodHandler::getRootDir_(
	std::map<std::string, std::vector<std::string> > const &location,
	Server const										  &server
) // clang-format on
{
	std::string rootdir = location.find("root") != location.end()
								  && !location.at("root").empty()
							  ? location.at("root")[0]
							  : server.getRoot();
	return rootdir;
}

// clang-format off
bool HttpMethodHandler::isCgiRequest_(
	std::map<std::string, std::vector<std::string> > const &location,
	std::string const									  &uri
)
{
	std::map<std::string, std::vector<std::string> >::const_iterator it
		= location.find("cgi"); // clang-format on
	std::cout << "Debug check\n" << std::endl;

	if (it != location.end() && !it->second.empty())
	{
		Logger::log(Logger::DEBUG)
			<< "CGI Extension: " << it->second[0] << std::endl;
		std::string cgiExtension = it->second[0]; // ".py"
		return uri.find(cgiExtension) != std::string::npos;
	}
	return false;
}

// clang-format off
std::string HttpMethodHandler::getCgiInterpreter_(
	std::map<std::string, std::vector<std::string> > const &location
) // clang-format on
{
	return location.find("cgi")->second[1]; // "/usr/bin/python3"
}

std::string HttpMethodHandler::handleCgiRequest_(
	std::string const &filepath,
	std::string const &interpreter,
	HttpRequest const &request,
	bool const		  &keepAlive,
	Server const	  &server,
	std::string const &rootdir,
	std::string const &uploadpath
)
{
	(void)uploadpath;
	Logger::log(Logger::INFO) << "Filepath: " << filepath << std::endl;
	Logger::log(Logger::INFO) << "Interpreter: " << interpreter << std::endl;

	int pipefd[2];
	if (pipe(pipefd) == -1)
	{
		Logger::log(Logger::ERROR, true) << "Pipe creation failed" << std::endl;
		return handleErrorResponse_(server, 500, rootdir, keepAlive);
	}

	pid_t pid = fork();
	if (pid == -1)
	{
		Logger::log(Logger::ERROR, true) << "Fork failed" << std::endl;
		return handleErrorResponse_(server, 500, rootdir, keepAlive);
	}
	else if (pid == 0)
	{
		// child process
		dup2(
			pipefd[0], STDIN_FILENO
		); // Redirect stdin to the read end of the pipe to receive body from
		   // parent process
		close(pipefd[0]);

		std::vector<std::string> envVariables;
		envVariables.push_back("GATEWAY_INTERFACE=CGI/1.1");
		envVariables.push_back("SERVER_PROTOCOL=HTTP/1.1");
		envVariables.push_back("REQUEST_METHOD=" + request.getMethod());
		envVariables.push_back("SCRIPT_FILENAME=" + filepath);
		envVariables.push_back("UPLOAD_PATH=" + uploadpath);
		envVariables.push_back(
			"CONTENT_LENGTH=" + ft::toString(request.getBody().size())
		);

		// clang-format off
		std::map<std::string, std::vector<std::string> > headers
			= request.getHeaders();
		for (std::map<std::string, std::vector<std::string> >::const_iterator it
			 = headers.begin();
			 it != headers.end();
			 ++it) // clang-format on
		{
			const std::string			   &headerKey = it->first;
			const std::vector<std::string> &headerValues = it->second;
			for (std::vector<std::string>::const_iterator valIt
				 = headerValues.begin();
				 valIt != headerValues.end();
				 ++valIt)
			{
				Logger::log(Logger::DEBUG) << "Debug Headers: " << headerKey
										   << ": " << *valIt << std::endl;
				envVariables.push_back(headerKey + "=" + *valIt);
			}
		}
		// clang-format off
		std::map<std::string, std::vector<std::string> > headers2
			= request.getHeaders();
		std::map<std::string, std::vector<std::string> >::iterator contentTypeIt
			= headers2.find("Content-Type"); // clang-format on
		if (contentTypeIt != headers2.end() && !contentTypeIt->second.empty())
		{
			std::string combinedContentType = contentTypeIt->second[0];
			for (size_t i = 1; i < contentTypeIt->second.size(); ++i)
			{
				combinedContentType += "; " + contentTypeIt->second[i];
			}
			envVariables.push_back("CONTENT_TYPE=" + combinedContentType);
		}

		std::vector<char *> envp;
		for (std::vector<std::string>::iterator it = envVariables.begin();
			 it != envVariables.end();
			 ++it)
		{
			envp.push_back(const_cast<char *>(it->c_str()));
		}
		envp.push_back(NULL);

		Logger::log(Logger::INFO)
			<< "Filepath before argv: " << filepath << std::endl;

		char *argv[]
			= {const_cast<char *>(interpreter.c_str()),
			   const_cast<char *>(filepath.c_str()),
			   NULL};

		dup2(pipefd[1], STDOUT_FILENO);

		if (execve(interpreter.c_str(), argv, &envp[0]) == -1)
		{
			Logger::log(Logger::ERROR, true)
				<< "Failed to execute CGI script: " << filepath << std::endl;
			close(pipefd[1]);
		}

		close(pipefd[1]);
		exit(EXIT_FAILURE);
	}
	else
	{
		// parent process

		// Write the request body to the pipe for the child process
		const std::vector<char> &requestBody = request.getBody();
		write(pipefd[1], requestBody.data(), requestBody.size());
		close(pipefd[1]); // Close the write end of the pipe after writing

		int status;
		waitpid(pid, &status, 0);
		if (status != 0)
		{
			Logger::log(Logger::ERROR, true)
				<< "CGI script execution failed" << std::endl;
			return HttpErrorHandler::getErrorPage(500);
		}

		char			  buffer[1024];
		std::stringstream output;
		ssize_t			  bytesRead;

		while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0)
			output.write(buffer, bytesRead);

		close(pipefd[0]); // Close the read end of the pipe after reading

		Logger::log(Logger::DEBUG)
			<< "CGI Output: " << output.str() << std::endl;

		HttpResponse response;
		response.setStatusCode(200);
		response.setReasonPhrase("OK");
		response.setHeader("Server", SERVER_NAME);
		response.setHeader("Date", ft::createTimestamp());
		response.setHeader("Content-Type", "text/html; charset=UTF-8");
		response.setHeader("Content-Length", ft::toString(output.str().size()));
		if (keepAlive)
			response.setHeader("Connection", "keep-alive");
		else
			response.setHeader("Connection", "close");
		response.setBody(output.str());

		return response.toString();
	}
}

bool HttpMethodHandler::isDirectory_(std::string const &filepath)
{
	struct stat fileStat;
	if (stat(filepath.c_str(), &fileStat) == 0)
		return S_ISDIR(fileStat.st_mode);
	return false;
}

// clang-format off
bool HttpMethodHandler::isAutoIndexEnabled_(
	const std::map<std::string, std::vector<std::string> > &location
) // clang-format on
{
	return location.find("autoindex") != location.end()
		   && location.at("autoindex")[0] == "on";
}

std::string HttpMethodHandler::handleAutoIndex_(
	std::string const &root,
	std::string const &uri,
	Server const	  &server,
	bool const		  &keepAlive
)
{
	Logger::log(Logger::DEBUG)
		<< "Handling auto index on: " << root << uri << std::endl;

	std::string body = generateAutoIndexPage_(root, uri, server, keepAlive);

	HttpResponse response;
	response.setStatusCode(200);
	response.setReasonPhrase("OK");
	response.setHeader("Server", SERVER_NAME);
	response.setHeader("Date", ft::createTimestamp());
	response.setHeader("Content-Type", "text/html; charset=UTF-8");
	response.setHeader("Content-Length", ft::toString(body.size()));
	if (keepAlive)
		response.setHeader("Connection", "keep-alive");
	else
		response.setHeader("Connection", "close");
	response.setBody(body);
	return response.toString();
}

std::string HttpMethodHandler::generateAutoIndexPage_(
	std::string const &root,
	std::string const &uri,
	Server const	  &server,
	bool const		  &keepAlive
)
{
	std::stringstream html;

	html << "<!DOCTYPE html>\n<html>\n<head><title>Index of " << uri
		 << "</title></head>\n<body><h1>Index of " << uri << "</h1>\n";
	html << "<ul>\n";

	if (uri != "/")
		html << "<li><a href=\"" << uri + "../\">Parent Directory</a></li>\n";

	DIR *dir = opendir((root + uri).c_str());
	if (dir == NULL)
	{
		Logger::log(Logger::ERROR, true)
			<< "Failed to open directory: " << root + uri << std::endl;
		return handleErrorResponse_(server, 405, root, keepAlive);
	}

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string filename(entry->d_name);

		// Skip "." and ".." directories
		if (filename == "." || filename == "..")
			continue;

		std::string fullPath = root + uri + "/" + filename;

		// Check if it is a directory
		struct stat fileStat;
		if (stat(fullPath.c_str(), &fileStat) == -1)
		{
			Logger::log(Logger::ERROR, true)
				<< "Failed to get file stats: " << fullPath << "Error: ["
				<< errno << "] " << strerror(errno) << std::endl;
			continue;
		}
		else if (S_ISDIR(fileStat.st_mode))
			filename += "/";

		// Generate links for each file/directory
		html << "<li><a href=\"" << uri << filename << "\">" << filename
			 << "</a></li>\n";
	}
	closedir(dir);
	html << "</ul>\n</body>\n</html>\n";

	return html.str();
}

// clang-format off
std::string HttpMethodHandler::findIndexFile_(
	const std::string									  &filepath,
	const std::map<std::string, std::vector<std::string> > &location,
	const Server										  &server
) // clang-format on
{
	std::vector<std::string> indexFiles
		= location.find("index") != location.end()
				  && !location.at("index").empty()
			  ? location.at("index")
			  : server.getIndex();

	struct stat fileStat;
	for (std::vector<std::string>::const_iterator it = indexFiles.begin();
		 it != indexFiles.end();
		 ++it)
	{
		std::string indexFilePath = filepath + "/" + *it;
		if (stat(indexFilePath.c_str(), &fileStat) == 0
			&& S_ISREG(fileStat.st_mode))
		{
			return indexFilePath;
		}
	}
	return "";
}

std::string HttpMethodHandler::createFileGetResponse_(
	std::string const &filepath,
	std::string const &rootdir,
	Server const	  &server,
	bool const		  &keepAlive
)
{
	HttpResponse  response;
	std::ifstream file(filepath.c_str());

	if (file.is_open())
	{
		Logger::log(Logger::DEBUG) << "Handling GET: file opened" << std::endl;

		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string body = buffer.str();

		response.setStatusCode(200);
		response.setReasonPhrase("OK");
		response.setHeader("Server", SERVER_NAME);
		response.setHeader("Date", ft::createTimestamp());
		response.setHeader("Content-Type", ft::getMimeType(filepath));
		response.setHeader("Content-Length", ft::toString(body.size()));
		if (keepAlive)
			response.setHeader("Connection", "keep-alive");
		else
			response.setHeader("Connection", "close");
		response.setBody(body);
	}
	else
	{
		Logger::log(Logger::DEBUG)
			<< "Handling GET: file not found" << std::endl;
		return handleErrorResponse_(server, 404, rootdir, keepAlive);
	}

	Logger::log(Logger::DEBUG) << "Handling GET: responding" << std::endl;
	return response.toString();
}

// clang-format off
std::string HttpMethodHandler::createFilePostResponse_(
	HttpRequest const							   &request,
	const std::string							   &rootdir,
	std::map<std::string, std::vector<std::string> > location,
	std::string const							   &uploadpath,
	Server const								   &server,
	bool const									   &keepAlive
) // clang-format on
{
	HttpResponse response;

	// Open the file for writing
	std::string	  uploadpathtmp = uploadpath + "/dummyfile";
	std::ofstream outFile;
	outFile.open(uploadpathtmp.c_str());
	if (!outFile)
	{
		Logger::log(Logger::ERROR)
			<< "Handling Post: failed to open file for writing: "
			<< uploadpathtmp << std::endl;
		return handleErrorResponse_(server, 500, rootdir, keepAlive);
	}

	// Write the request body to the file
	const std::vector<char> &requestBody = request.getBody();
	outFile.write(requestBody.data(), requestBody.size());
	if (!outFile)
	{
		Logger::log(Logger::ERROR)
			<< "handling Post: failed to write to file: " << uploadpathtmp
			<< std::endl;
		return handleErrorResponse_(server, 500, rootdir, keepAlive);
	}

	outFile.close();

	// Check for redirections
	std::string redirection = handleRedirection_(location, keepAlive);
	if (!redirection.empty())
		return redirection;

	// Generate a success response
	response.setStatusCode(200);
	response.setReasonPhrase("OK");
	response.setHeader("Server", SERVER_NAME);
	response.setHeader("Date", ft::createTimestamp());
	response.setHeader("Content-Type", "text/html; charset=UTF-8");
	std::string responseBody = "<h1>File Uploaded Successfully</h1>\n";
	response.setHeader("Content-Length", ft::toString(responseBody.size()));
	if (keepAlive)
		response.setHeader("Connection", "keep-alive");
	else
		response.setHeader("Connection", "close");
	response.setBody(responseBody);

	Logger::log(Logger::DEBUG) << "Handling POST: responding" << std::endl;
	return response.toString();
}

// clang-format off
std::string HttpMethodHandler::createDeleteResponse_(
	const std::string							   &filepath,
	const std::string							   &rootdir,
	std::map<std::string, std::vector<std::string> > location,
	const Server								   &server,
	bool											keepAlive
) // clang-format on
{
	std::string	 body;
	HttpResponse response;

	if (remove(filepath.c_str()) == 0)
	{
		// Check for redirections
		std::string redirection = handleRedirection_(location, keepAlive);
		if (!redirection.empty())
			return redirection;

		response.setStatusCode(200);
		response.setReasonPhrase("OK");
		body = "<h1>File Deleted Successfully</h1>\n";
	}
	else
	{
		Logger::log(Logger::DEBUG)
			<< "Handling DELETE: file could not be deleted or not found"
			<< std::endl;

		// Permission denied
		if (errno == EACCES)
			return handleErrorResponse_(server, 403, rootdir, keepAlive);
		// File not found
		else if (errno == ENOENT)
			return handleErrorResponse_(server, 404, rootdir, keepAlive);
		// Server error (500)
		else
			return handleErrorResponse_(server, 500, rootdir, keepAlive);
	}
	response.setHeader("Server", SERVER_NAME);
	response.setHeader("Date", ft::createTimestamp());
	response.setHeader("Content-Type", "text/html; charset=UTF-8");
	response.setHeader("Content-Length", ft::toString(body.size()));
	if (keepAlive)
		response.setHeader("Connection", "keep-alive");
	else
		response.setHeader("Connection", "close");
	response.setBody(body);

	Logger::log(Logger::DEBUG) << "Handling DELETE: responding" << std::endl;
	return response.toString();
}
