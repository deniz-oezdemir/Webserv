#pragma once

#define DEFAULT_PORT 80
#define QUEUE_SIZE	 1
#define POLL_TIMEOUT -1
#define BUFFER_SIZE	 8192

#define HTTP_ACCEPTED_METHODS                                                  \
	{                                                                          \
		"GET", "POST", "DELETE"                                                \
	}

// HTTP CODES
#define HTTP_200_CODE	200
#define HTTP_200_REASON "OK"
#define HTTP_301_CODE	301
#define HTTP_301_REASON "Moved Permanently"
#define HTTP_400_CODE	400
#define HTTP_400_REASON "Bad Request"
#define HTTP_403_CODE	403
#define HTTP_403_REASON "Forbidden"
#define HTTP_404_CODE	404
#define HTTP_404_REASON "Not Found"
#define HTTP_405_CODE	405
#define HTTP_405_REASON "Method Not Allowed"
#define HTTP_413_CODE	413
#define HTTP_413_REASON "Content Too Large"
#define HTTP_414_CODE	414
#define HTTP_414_REASON "URI Too Long"
#define HTTP_500_CODE	500
#define HTTP_500_REASON "Internal Server Error"
#define HTTP_501_CODE	501
#define HTTP_501_REASON "Not Implemented"

// HTTP HEADERS
#define CONTENT_LENGTH	  "Content-Length"
#define CONTENT_TYPE	  "Content-Type"
#define HOST			  "Host"
#define TRANSFER_ENCODING "Transfer-Encoding"
