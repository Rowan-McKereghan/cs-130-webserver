#include "request_handler_static.h"

#include <boost/filesystem.hpp>

RequestHandlerStatic::RequestHandlerStatic(boost::filesystem::path root) : root_(root) {}

// TODO - implement
void RequestHandlerStatic::HandleRequest(const Request &req, Response &res) {
    if(root_.string().compare("Invalid Path")) {
        res.status_code = 404;
        return;
    }

    HTTPHeader content_type_header;
    content_type_header.name = "Content-Type";

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
        res.status_code = 404;
        return;
    }


    res.status_code = 200;
    if(uri.extension().string().compare(".txt")) {
        content_type_header.value = "text/plain";
    }
    else if(uri.extension().string().compare(".jpg") || uri.extension().string().compare(".jpeg")) {
        content_type_header.value = "image/jpeg";
    }
    else if(uri.extension().string().compare(".png")) {
        content_type_header.value = "image/png";
    }
    else if(uri.extension().string().compare(".pdf")) {
        content_type_header.value = "application/pdf";
    }
    else if(uri.extension().string().compare(".html")) {
        content_type_header.value = "text/html";
    }
    else if(uri.extension().string().compare(".zip")) {
        content_type_header.value = "application/zip";
    }
    else {
        content_type_header.value = "application/octet-stream"; //download file as binary if it exists but our server doesn't support it
    }
    res.headers.push_back(content_type_header);

    HTTPHeader content_length_header;
    content_length_header.name = "Content-Length";
    content_length_header.value = std::to_string(boost::filesystem::file_size(complete_path));
    res.headers.push_back(content_length_header);

    


}