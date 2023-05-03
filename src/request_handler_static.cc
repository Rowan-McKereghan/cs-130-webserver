#include "request_handler_static.h"

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <map>
#include <sstream>
#include <iostream>

#include "logger.h"

RequestHandlerStatic::RequestHandlerStatic(boost::filesystem::path root) : root_(root) {}

int RequestHandlerStatic::HandleRequestHelper(const Request &req, boost::filesystem::path &complete_path, Response &res) {
    HTTPHeader content_type_header;
    content_type_header.name = "Content-Type";
    HTTPHeader content_length_header;
    content_length_header.name = "Content-Length";

    if(!root_.string().compare("Invalid Path")) {
        res.data = "404 Not Found\r\n\r\n";
        res.status_code = NOT_FOUND;
        content_type_header.value = "text/HTML";
        content_length_header.value = res.data.length();
        std::vector<HTTPHeader> headers;
        res.headers.push_back(content_type_header);
        res.headers.push_back(content_length_header);
        res.status_code = NOT_FOUND;
        return -1;
    }
    boost::filesystem::path uri{req.uri};
    uri = uri.relative_path(); //strip /static/ from uri
    if (uri.empty()) {
        uri = {""};
    }
    else {
        uri = uri.lexically_relative(*uri.begin());
    }
    complete_path = root_ / uri; //concatenate root path and uri for actual path

    if (!boost::filesystem::is_regular_file(complete_path)) { //im not sure about the case when we serve symlinks... does it work the same?
        LOG(error) << "File not found";
        res.data = "404 Not Found\r\n\r\n";
        res.status_code = NOT_FOUND;
        content_type_header.value = "text/HTML";
        content_length_header.value = res.data.length();
        std::vector<HTTPHeader> headers;
        res.headers.push_back(content_type_header);
        res.headers.push_back(content_length_header);
        res.status_code = NOT_FOUND;
        return -1;
    }

    if (boost::filesystem::file_size(complete_path) > MAX_FILE_SIZE - 1) { 
        LOG(error) << "File requested too large (more than 10MB)";
        res.status_code = 400;
        return -1;
    }

    LOG(info) << "Serving file " << complete_path.string();
    res.is_serving_file = true;

    //set MIME type
    res.status_code = 200;
    if(!uri.extension().string().compare(".txt")) {
        content_type_header.value = "text/plain";
    }
    else if(!uri.extension().string().compare(".jpg") || !uri.extension().string().compare(".jpeg")) {
        content_type_header.value = "image/jpeg";
    }
    else if(!uri.extension().string().compare(".png")) {
        content_type_header.value = "image/png";
    }
    else if(!uri.extension().string().compare(".pdf")) {
        content_type_header.value = "application/pdf";
    }
    else if(!uri.extension().string().compare(".html") || !uri.extension().string().compare(".htm")) {
        content_type_header.value = "text/html";
    }
    else if(!uri.extension().string().compare(".zip")) {
        content_type_header.value = "application/zip";
    }
    else {
        content_type_header.value = "application/octet-stream"; //download file as binary if it exists but our server doesn't support its type
    }
    res.headers.push_back(content_type_header);

    //find file size and set content length field
    content_length_header.value = std::to_string(boost::filesystem::file_size(complete_path));
    res.headers.push_back(content_length_header);
    return 0;
}

void RequestHandlerStatic::HandleRequest(const Request &req, Response &res) {

    boost::filesystem::path complete_path;

    if(HandleRequestHelper(req, complete_path, res) == -1) {
        return;
    }

    std::string response = res.generate_http_response();
    res.write_to_socket(response);


    std::ifstream file_stream(complete_path.string(), std::ios::in | std::ios::binary);
    file_stream.seekg(0, std::ios::beg);

    // Write the file contents to the socket in chunks
    char buffer_arr[8192];
    while (file_stream) {
        file_stream.read(buffer_arr, sizeof(buffer_arr));
        boost::system::error_code ec;
        boost::asio::write(*res.socket_, boost::asio::buffer(buffer_arr), ec);
        if (ec) {
            log_error(ec, "Failed to write requested file to socket");
            return;
        }
    }
}