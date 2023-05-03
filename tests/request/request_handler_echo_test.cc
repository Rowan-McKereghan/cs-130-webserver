#include "gtest/gtest.h"
#include "request_handler_echo.h"

TEST(RequestHandlerEcho, EchoTest) {
	boost::asio::ip::tcp::socket* socket;
	string req_data = "Sample data to be echoed";
	Request req(req_data);
	req.method = "GET";
	req.uri = "/echo";
	req.http_version = 1;
	
	Response res(socket);
	RequestHandlerEcho handler;
	handler.HandleRequest(req, res);
	EXPECT_EQ(res.data, req.raw_request);
	EXPECT_EQ(res.headers.size(), 2);
	EXPECT_EQ(res.status_code, OK);
	
}