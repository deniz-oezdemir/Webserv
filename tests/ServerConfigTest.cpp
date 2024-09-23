#include "test.hpp"

Test(ServerConfig, CorrectPathArgument)
{
	try
	{
		ServerConfig config("test.config");
		cr_assert(eq(int, config.getFile().is_open(), true));
	}
	catch (std::exception &e)
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
	catch (std::exception &e)
	{
		cr_assert(
			eq(str,
			   e.what(),
			   "Could not open the file [../fake.config] : (2) No such file or "
			   "directory")
		);
	}
}

Test(ServerConfig, CorrectConfig)
{
	try
	{
		ServerConfig config("test.config");
		config.parseFile(false, false);
		cr_assert(eq(int, config.isConfigOK(), 1));
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		cr_fail();
	}
}

Test(ServerConfig, CheckGeneralDirectives)
{
	try
	{
		ServerConfig config("test.config");
		config.parseFile(false, false);
		std::string errorLog = config.getGeneralConfigValue("error_log");
		std::string workerProcesses
			= config.getGeneralConfigValue("worker_processes");
		std::string workerConnections
			= config.getGeneralConfigValue("worker_connections");
		cr_assert(eq(str, errorLog, "debug"));
		cr_assert(eq(str, workerProcesses, "auto"));
		cr_assert(eq(str, workerConnections, "1024"));
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		cr_fail();
	}
}

Test(ServerConfig, CheckServerDirectives)
{
	try
	{
		ServerConfig config("test.config");
		config.parseFile(false, false);
		ConfigValue value;

		if (!config.getServerConfigValue(0, "listen", value))
			throw std::runtime_error("Could not find the key [listen] in the "
									 "server[0] configuration map.");
		std::vector<std::string> tmp = value.getMapValue("host");
		cr_assert(eq(str, tmp[0], "0.0.0.0"));
		cr_assert(eq(str, tmp[1], "0.0.0.0"));
		tmp = value.getMapValue("port");
		cr_assert(eq(str, tmp[0], "8080"));
		cr_assert(eq(str, tmp[1], "8181"));

		if (!config.getServerConfigValue(0, "server_name", value))
			throw std::runtime_error("Could not find the key [server_name] in "
									 "the server[0] configuration map.");
		cr_assert(
			eq(str,
			   const_cast<std::string &>(value.getVectorValue(0)),
			   "example.com")
		);
		cr_assert(
			eq(str,
			   const_cast<std::string &>(value.getVectorValue(1)),
			   "www.example.com")
		);

		if (!config.getServerConfigValue(0, "client_max_body_size", value))
			throw std::runtime_error(
				"Could not find the key [client_max_body_size] in the "
				"server[0] configuration map."
			);
		cr_assert(
			eq(str,
			   const_cast<std::string &>(value.getVectorValue(0)),
			   "1000000000")
		);

		if (!config.getServerConfigValue(0, "root", value))
			throw std::runtime_error("Could not find the key [root] in the "
									 "server[0] configuration map.");
		cr_assert(eq(str, const_cast<std::string &>(value.getVectorValue(0)),
				 "../www/website"));

		if (!config.getServerConfigValue(0, "index", value))
			throw std::runtime_error("Could not find the key [index] in the "
								 "server[0] configuration map.");
		cr_assert(
			eq(str,
			   const_cast<std::string &>(value.getVectorValue(0)),
			   "index.html")
		);
		cr_assert(
			eq(str,
			   const_cast<std::string &>(value.getVectorValue(1)),
			   "index.htm")
		);

		if (!config.getServerConfigValue(0, "404", value))
			throw std::runtime_error("Could not find the key [error_page] in the "
								 "server[0] configuration map.");
		cr_assert(
			eq(str,
			   const_cast<std::string &>(value.getVectorValue(0)),
			   "/404.html")
		);

		if (!config.getServerConfigValue(0, "503", value))
			throw std::runtime_error("Could not find the key [error_page] in the "
							 "server[0] configuration map.");
		cr_assert(
			eq(str,
			   const_cast<std::string &>(value.getVectorValue(0)),
			   "/5xx.html")
		);

		if (!config.getServerConfigValue(0, "/", value))
			throw std::runtime_error("Could not find the key [location] in the "
							 "server[0] configuration map.");
		tmp = value.getMapValue("limit_except");
		cr_assert(eq(str, tmp[0], "GET"));
		cr_assert(eq(int, tmp.size(), 1));

		if (!config.getServerConfigValue(0, "/some-images/", value))
			throw std::runtime_error("Could not find the key [location] in the "
						 "server[0] configuration map.");
		tmp = value.getMapValue("root");
		cr_assert(eq(str, tmp[0], "/"));
		cr_assert(eq(int, tmp.size(), 1));
		tmp = value.getMapValue("autoindex");
		cr_assert(eq(str, tmp[0], "on"));
		cr_assert(eq(int, tmp.size(), 1));

		if (!config.getServerConfigValue(0, "/with-cgi", value))
			throw std::runtime_error("Could not find the key [location] in the "
					 "server[0] configuration map.");
		tmp = value.getMapValue("root");
		cr_assert(eq(str, tmp[0], "/"));
		cr_assert(eq(int, tmp.size(), 1));
		tmp = value.getMapValue("cgi");
		cr_assert(eq(str, tmp[0], ".py"));
		cr_assert(eq(str, tmp[1], "/usr/bin/python3"));
		cr_assert(eq(int, tmp.size(), 2));

		if (!config.getServerConfigValue(0, "/upload/", value))
			throw std::runtime_error("Could not find the key [location] in the "
				 "server[0] configuration map.");
		tmp = value.getMapValue("limit_except");
		cr_assert(eq(str, tmp[0], "POST"));
		cr_assert(eq(int, tmp.size(), 1));
		tmp = value.getMapValue("upload_store");
		cr_assert(eq(str, tmp[0], "/var/www/uploads"));
		
		if (!config.getServerConfigValue(0, "/old-page", value))
			throw std::runtime_error("Could not find the key [location] in the "
			 "server[0] configuration map.");
		tmp = value.getMapValue("return");
		cr_assert(eq(str, tmp[0], "301"));
		cr_assert(eq(str, tmp[1], "/new-page"));
		cr_assert(eq(int, tmp.size(), 2));
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		cr_fail();
	}
}
