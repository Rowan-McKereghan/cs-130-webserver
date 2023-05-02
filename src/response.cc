#include "response.h"
#include <time.h>

std::string Response::generate_http_response() {
	// TODO: generate the actual http_response with the content body

	time_t t;
	struct tm* myTime;
	char date[100];
    std::string res = "HTTP/1.1 200 OK\n";
    t = time(NULL);
    myTime = gmtime(&t);
    strftime(date, sizeof(date), "Date: %a, %d %b %G %T GMT", myTime);
    std::string addDate(date);
    res += addDate + "\n";
    res += "Server: webserver\n";
    // TODO: Add a field: "Content-Length: " + body length in bytes+ "\n"; to
    // response header when we need to deal with message bodies. optional in 1.1
    // but not in other versions
    res += "Content-Type: text/plain\n";
    res += "\r\n";
	return res;
}