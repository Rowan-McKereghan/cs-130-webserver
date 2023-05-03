#include "gtest/gtest.h"
#include "request_handler_static.h"

TEST(RequestHandlerStatic, StaticTest404) {
	boost::asio::ip::tcp::socket* socket;
	string req_data = "Irrelevant stuff in request body";
	Request req(req_data);
	req.method = "GET";
	req.uri = "/static/file_does_not_exist.fake-extension";
	req.http_version = 1;
	
	Response res(socket);
    boost::filesystem::path root{"/usr/src/projects/"};
    boost::filesystem::path complete_path;
	RequestHandlerStatic handler(root);
	handler.HandleRequestHelper(req, res);
	EXPECT_EQ(res.headers.size(), 2);
	EXPECT_EQ(res.status_code, NOT_FOUND);
}

TEST(RequestHandlerStatic, StaticTestTxtFile) {
	boost::asio::ip::tcp::socket* socket;
	string req_data = "Irrelevant stuff in request body";
	Request req(req_data);
	req.method = "GET";
	req.uri = "/static/sample.txt";
	req.http_version = 1;
	
	Response res(socket);
    boost::filesystem::path root{"/usr/src/projects/ctrl-c-ctrl-v/tests/static_test_files/sample.txt"};
	RequestHandlerStatic handler(root);
	handler.HandleRequestHelper(req, res);
	EXPECT_EQ(res.headers.size(), 2);
	EXPECT_EQ(res.status_code, OK);
    EXPECT_EQ(res.headers[0].value, "text/plain");
}

TEST(RequestHandlerStatic, StaticTestPngFile) {
	boost::asio::ip::tcp::socket* socket;
	string req_data = "Irrelevant stuff in request body";
	Request req(req_data);
	req.method = "GET";
	req.uri = "/static/sample.png";
	req.http_version = 1;
	
	Response res(socket);
    boost::filesystem::path root{"/usr/src/projects/ctrl-c-ctrl-v/tests/static_test_files/sample.png"};
	RequestHandlerStatic handler(root);
	handler.HandleRequestHelper(req, res);
	EXPECT_EQ(res.headers.size(), 2);
	EXPECT_EQ(res.status_code, OK);
    EXPECT_EQ(res.headers[0].value, "image/png");
}