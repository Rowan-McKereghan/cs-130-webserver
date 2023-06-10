#include "crud_handler.h"

#include <boost/beast/core.hpp>

#include "crud_filesystem_manager.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class MockManager : public CrudFileSystemManager {
 public:
  MockManager() : CrudFileSystemManager(){};
  MOCK_METHOD(bool, CreateDir, (boost::filesystem::path), (override));
  MOCK_METHOD(bool, CreateFile, (boost::filesystem::path path, std::string content), (override));
  MOCK_METHOD(bool, WriteFile, (boost::filesystem::path path, std::string content), (override));
  MOCK_METHOD(bool, DeleteFile, (boost::filesystem::path path), (override));
  MOCK_METHOD(bool, ReadFile,
              (boost::filesystem::path path, boost::beast::http::response<boost::beast::http::dynamic_body>& res),
              (override));
  MOCK_METHOD(bool, ListFiles, (boost::filesystem::path path, std::vector<std::string>& out_vect), (override));
};

class CrudHandlerTest : public ::testing::Test {
 protected:
  unordered_map<std::string, std::unordered_set<int>> file_to_id;
  MockManager manager;
  boost::beast::http::response<boost::beast::http::dynamic_body> res;

  void SetUp() override {}
};

TEST_F(CrudHandlerTest, GetNextIDNewEntity) {
  std::string file_path = "/crud";
  CrudHandler handler(file_path, file_to_id, &manager);

  int res = handler.GetNextID("entity");
  EXPECT_EQ(res, 1);
}

TEST_F(CrudHandlerTest, GetNextIDNormal) {
  std::string file_path = "/crud";
  file_to_id["entity"].insert(1);
  file_to_id["entity"].insert(2);

  CrudHandler handler(file_path, file_to_id, &manager);

  int res = handler.GetNextID("entity");
  EXPECT_EQ(res, 3);
}

TEST_F(CrudHandlerTest, GetNextIDDeletedNum) {
  std::string file_path = "/crud";
  file_to_id["entity"].insert(1);
  file_to_id["entity"].insert(3);
  CrudHandler handler(file_path, file_to_id, &manager);

  int res = handler.GetNextID("entity");
  EXPECT_EQ(res, 2);
}

TEST_F(CrudHandlerTest, ParseTargetNoID) {
  std::string file_path = "/crud/entity";

  CrudHandler handler(file_path, file_to_id, &manager);

  std::pair<std::string, int> res = handler.ParseTarget();
  EXPECT_EQ(res.first, "/crud/entity");
  EXPECT_EQ(res.second, -1);
}

TEST_F(CrudHandlerTest, ParseTargetYesID) {
  std::string file_path = "/crud/entity/1";

  CrudHandler handler(file_path, file_to_id, &manager);

  std::pair<std::string, int> res = handler.ParseTarget();
  EXPECT_EQ(res.first, "/crud/entity");
  EXPECT_EQ(res.second, 1);
}

TEST_F(CrudHandlerTest, ParseTargetDoubleDigitID) {
  std::string file_path = "/crud/entity/12";

  CrudHandler handler(file_path, file_to_id, &manager);

  std::pair<std::string, int> res = handler.ParseTarget();
  EXPECT_EQ(res.first, "/crud/entity");
  EXPECT_EQ(res.second, 12);
}

TEST_F(CrudHandlerTest, ParseTargetIDHasNonDigits) {
  std::string file_path = "/crud/entity/1f";

  CrudHandler handler(file_path, file_to_id, &manager);

  std::pair<std::string, int> res = handler.ParseTarget();
  EXPECT_EQ(res.first, "/crud/entity/1f");
  EXPECT_EQ(res.second, -1);
}

// tests HandleRequest with an unsupported HTTP method
TEST_F(CrudHandlerTest, CrudTestUnsuportedMethod) {
  string req_data = "Irrelevant stuff in request body";
  boost::beast::http::request<boost::beast::http::string_body> req{
      boost::beast::http::verb::connect,  // Unsupported method
      "/api/entity/1",                    // URI
      11};                                // HTTP 1.1
  req.body() = req_data;

  std::string file_path = "/crud/entity/1";

  EXPECT_CALL(manager, CreateDir(testing::_)).WillOnce(testing::Return(true));

  CrudHandler handler(file_path, file_to_id, &manager);
  handler.HandleRequest(req, res);

  EXPECT_EQ(res.version(), 11);                                                          // version
  EXPECT_EQ(res.result_int(), BAD_REQUEST);                                              // status
  EXPECT_EQ(boost::beast::buffers_to_string(res.body().data()), "400 Bad Request\n\n");  // body
}

// tests HandleRequest using Post method
TEST_F(CrudHandlerTest, CrudTestPost) {
  // set up Crud handler for post
  std::string file_path = "/crud/entity";

  EXPECT_CALL(manager, CreateDir(testing::_)).WillOnce(testing::Return(true)).WillOnce(testing::Return(true));
  EXPECT_CALL(manager, CreateFile(testing::_, testing::_)).WillOnce(testing::Return(true));

  CrudHandler handler(file_path, file_to_id, &manager);

  // set up post request
  string req_data = "{json}";
  boost::beast::http::request<boost::beast::http::string_body> req_post{boost::beast::http::verb::post,  // POST REQUEST
                                                                        "/api/entity",                   // URI
                                                                        11};                             // HTTP 1.1
  req_post.body() = req_data;

  handler.HandleRequest(req_post, res);

  // check post response
  EXPECT_EQ(res.version(), 11);                                                      // version
  EXPECT_EQ(res.result_int(), OK);                                                   // status
  EXPECT_EQ(boost::beast::buffers_to_string(res.body().data()), "{\"id\": 1}\n\n");  // body
}

// tests HandleRequest using bad Post request with ID included
TEST_F(CrudHandlerTest, CrudTestPostWithID) {
  // set up Crud handler for post
  std::string file_path = "/crud/entity/1";

  EXPECT_CALL(manager, CreateDir(testing::_)).WillOnce(testing::Return(true));
  CrudHandler handler(file_path, file_to_id, &manager);

  // set up post request
  string req_data = "{json}";
  boost::beast::http::request<boost::beast::http::string_body> req_post{boost::beast::http::verb::post,  // POST REQUEST
                                                                        "/api/entity/1",                 // URI
                                                                        11};                             // HTTP 1.1
  req_post.body() = req_data;

  handler.HandleRequest(req_post, res);

  // check post response
  EXPECT_EQ(res.version(), 11);                                                          // version
  EXPECT_EQ(res.result_int(), BAD_REQUEST);                                              // status
  EXPECT_EQ(boost::beast::buffers_to_string(res.body().data()), "400 Bad Request\n\n");  // body
}

// tests HandleRequest using Put method
TEST_F(CrudHandlerTest, CrudTestPut) {
  file_to_id["/crud/entity"].insert(1);

  // set up Crud handler for put
  std::string file_path = "/crud/entity/1";

  EXPECT_CALL(manager, CreateDir(testing::_)).WillOnce(testing::Return(true));
  EXPECT_CALL(manager, WriteFile(testing::_, testing::_)).WillOnce(testing::Return(true));

  CrudHandler handler(file_path, file_to_id, &manager);

  // set up put request
  string req_data = "{json}";
  boost::beast::http::request<boost::beast::http::string_body> req_put{boost::beast::http::verb::put,  // POST REQUEST
                                                                       "/api/entity/1",                // URI
                                                                       11};                            // HTTP 1.1
  req_put.body() = req_data;

  handler.HandleRequest(req_put, res);

  // check put response
  EXPECT_EQ(res.version(), 11);                                                      // version
  EXPECT_EQ(res.result_int(), OK);                                                   // status
  EXPECT_EQ(boost::beast::buffers_to_string(res.body().data()), "{\"id\": 1}\n\n");  // body
}

// tests HandleRequest using Put method for nonexistant item
TEST_F(CrudHandlerTest, CrudTestPutNotFound) {
  // set up Crud handler for put
  std::string file_path = "/crud/entity/1";

  EXPECT_CALL(manager, CreateDir(testing::_)).WillOnce(testing::Return(true));

  CrudHandler handler(file_path, file_to_id, &manager);

  // set up put request
  string req_data = "{json}";
  boost::beast::http::request<boost::beast::http::string_body> req_put{boost::beast::http::verb::put,  // POST REQUEST
                                                                       "/api/entity/1",                // URI
                                                                       11};                            // HTTP 1.1
  req_put.body() = req_data;

  handler.HandleRequest(req_put, res);

  // check put response
  EXPECT_EQ(res.version(), 11);                                                        // version
  EXPECT_EQ(res.result_int(), NOT_FOUND);                                              // status
  EXPECT_EQ(boost::beast::buffers_to_string(res.body().data()), "404 Not Found\n\n");  // body
}

// tests HandleRequest using Put method with no ID returns 400 error
TEST_F(CrudHandlerTest, CrudTestPutNoID) {
  // set up Crud handler for put
  std::string file_path = "/crud/entity";

  EXPECT_CALL(manager, CreateDir(testing::_)).WillOnce(testing::Return(true));

  CrudHandler handler(file_path, file_to_id, &manager);

  // set up put request
  string req_data = "{json}";
  boost::beast::http::request<boost::beast::http::string_body> req_put{boost::beast::http::verb::put,  // POST REQUEST
                                                                       "/api/entity",                  // URI
                                                                       11};                            // HTTP 1.1
  req_put.body() = req_data;

  handler.HandleRequest(req_put, res);

  // check put response
  EXPECT_EQ(res.version(), 11);                                                          // version
  EXPECT_EQ(res.result_int(), BAD_REQUEST);                                              // status
  EXPECT_EQ(boost::beast::buffers_to_string(res.body().data()), "400 Bad Request\n\n");  // body
}

// tests HandleRequest using Delete method
TEST_F(CrudHandlerTest, CrudTestDelete) {
  file_to_id["/crud/entity"].insert(1);

  // set up Crud handler
  std::string file_path = "/crud/entity/1";

  EXPECT_CALL(manager, CreateDir(testing::_)).WillOnce(testing::Return(true));
  EXPECT_CALL(manager, DeleteFile(testing::_)).WillOnce(testing::Return(true));

  CrudHandler handler(file_path, file_to_id, &manager);

  // set up request
  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::delete_,  // DELETE REQUEST
                                                                   "/api/entity/1",                    // URI
                                                                   11};                                // HTTP 1.1

  handler.HandleRequest(req, res);

  // check response
  EXPECT_EQ(res.version(), 11);                                                      // version
  EXPECT_EQ(res.result_int(), OK);                                                   // status
  EXPECT_EQ(boost::beast::buffers_to_string(res.body().data()), "{\"id\": 1}\n\n");  // body

  // check that id was removed from set
  bool wasRemoved = file_to_id["/crud/entity"].find(1) == file_to_id["/crud/entity"].end();
  EXPECT_TRUE(wasRemoved);
}

// tests HandleRequest using Delete method with nonexistant entity returns 404 error
TEST_F(CrudHandlerTest, CrudTestDeleteNotFound) {
  // set up Crud handler
  std::string file_path = "/crud/entity/1";

  EXPECT_CALL(manager, CreateDir(testing::_)).WillOnce(testing::Return(true));

  CrudHandler handler(file_path, file_to_id, &manager);

  // set up request
  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::delete_,  // DELETE REQUEST
                                                                   "/api/entity/1",                    // URI
                                                                   11};                                // HTTP 1.1

  handler.HandleRequest(req, res);

  // check response
  EXPECT_EQ(res.version(), 11);                                                        // version
  EXPECT_EQ(res.result_int(), NOT_FOUND);                                              // status
  EXPECT_EQ(boost::beast::buffers_to_string(res.body().data()), "404 Not Found\n\n");  // body
}

// tests HandleRequest using Delete method without ID returns 400 error
TEST_F(CrudHandlerTest, CrudTestDeleteNoID) {
  // set up Crud handler
  std::string file_path = "/crud/entity";

  EXPECT_CALL(manager, CreateDir(testing::_)).WillOnce(testing::Return(true));

  CrudHandler handler(file_path, file_to_id, &manager);

  // set up request
  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::delete_,  // DELETE REQUEST
                                                                   "/api/entity",                      // URI
                                                                   11};                                // HTTP 1.1

  handler.HandleRequest(req, res);

  // check response
  EXPECT_EQ(res.version(), 11);                                                          // version
  EXPECT_EQ(res.result_int(), BAD_REQUEST);                                              // status
  EXPECT_EQ(boost::beast::buffers_to_string(res.body().data()), "400 Bad Request\n\n");  // body
}

// tests HandleRequest using Get method using ID
TEST_F(CrudHandlerTest, CrudTestGetID) {
  file_to_id["/crud/entity"].insert(1);

  // set up Crud handler
  std::string file_path = "/crud/entity/1";

  EXPECT_CALL(manager, CreateDir(testing::_)).WillOnce(testing::Return(true));
  boost::beast::http::response<boost::beast::http::dynamic_body> test_res;
  boost::beast::ostream(test_res.body()) << "{json}\n\n";
  EXPECT_CALL(manager, ReadFile(testing::_, testing::_))
      .WillOnce(testing::DoAll(testing::SetArgReferee<1>(test_res), testing::Return(true)));

  CrudHandler handler(file_path, file_to_id, &manager);

  // set up request
  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get,  // GET REQUEST
                                                                   "/api/entity/1",                // URI
                                                                   11};                            // HTTP 1.1

  handler.HandleRequest(req, res);

  // check response
  EXPECT_EQ(res.version(), 11);                                                 // version
  EXPECT_EQ(res.result_int(), OK);                                              // status
  EXPECT_EQ(boost::beast::buffers_to_string(res.body().data()), "{json}\n\n");  // body
}

// tests HandleRequest using Get method without ID to list files
TEST_F(CrudHandlerTest, CrudTestGetNoID) {
  file_to_id["/crud/entity"].insert(1);
  file_to_id["/crud/entity"].insert(2);

  // set up Crud handler

  std::string file_path = "/crud/entity";

  EXPECT_CALL(manager, CreateDir(testing::_)).WillOnce(testing::Return(true));
  std::vector<std::string> file_names{"1", "2"};
  EXPECT_CALL(manager, ListFiles(testing::_, testing::_))
      .WillOnce(testing::DoAll(testing::SetArgReferee<1>(file_names), testing::Return(true)));

  CrudHandler handler(file_path, file_to_id, &manager);

  // set up request
  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get,  // GET REQUEST
                                                                   "/api/entity",                  // URI
                                                                   11};                            // HTTP 1.1
  handler.HandleRequest(req, res);

  // check response
  EXPECT_EQ(res.version(), 11);                                                 // version
  EXPECT_EQ(res.result_int(), OK);                                              // status
  EXPECT_EQ(boost::beast::buffers_to_string(res.body().data()), "[1, 2]\n\n");  // body
}

// tests HandleRequest using Get method with an entity that does not exist
TEST_F(CrudHandlerTest, CrudTestGetNonexistantEntity) {
  // set up Crud handler

  std::string file_path = "/crud/entity";

  EXPECT_CALL(manager, CreateDir(testing::_)).WillOnce(testing::Return(true));

  CrudHandler handler(file_path, file_to_id, &manager);

  // set request
  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get,  // GET REQUEST
                                                                   "/api/entity",                  // URI
                                                                   11};                            // HTTP 1.1
  handler.HandleRequest(req, res);

  // check response
  EXPECT_EQ(res.version(), 11);                                             // version
  EXPECT_EQ(res.result_int(), OK);                                          // status
  EXPECT_EQ(boost::beast::buffers_to_string(res.body().data()), "[]\n\n");  // body
}

// tests HandleRequest using Get method with an entity and ID that does not exist
TEST_F(CrudHandlerTest, CrudTestGetNotFound) {
  // set up Crud handler

  std::string file_path = "/crud/entity/1";

  EXPECT_CALL(manager, CreateDir(testing::_)).WillOnce(testing::Return(true));

  CrudHandler handler(file_path, file_to_id, &manager);

  // set up request
  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get,  // GET REQUEST
                                                                   "/api/entity",                  // URI
                                                                   11};                            // HTTP 1.1
  handler.HandleRequest(req, res);

  // check put response
  EXPECT_EQ(res.version(), 11);                                                        // version
  EXPECT_EQ(res.result_int(), NOT_FOUND);                                              // status
  EXPECT_EQ(boost::beast::buffers_to_string(res.body().data()), "404 Not Found\n\n");  // body
}

TEST_F(CrudHandlerTest, CrudReturnErrorNotFound) {
  // set up Crud handler

  std::string file_path = "/crud/entity/1";

  CrudHandler handler(file_path, file_to_id, &manager);
  handler.ReturnError(static_cast<StatusCode>(418), res);

  EXPECT_EQ(boost::beast::buffers_to_string(res.body().data()), "418 Unknown Status Code\n\n");
}

TEST_F(CrudHandlerTest, CrudFailedCreateDir) {
  // set up Crud handler for post
  std::string file_path = "/crud/entity";

  EXPECT_CALL(manager, CreateDir(testing::_)).WillOnce(testing::Return(true)).WillOnce(testing::Return(false));

  CrudHandler handler(file_path, file_to_id, &manager);

  // set up post request
  string req_data = "{json}";
  boost::beast::http::request<boost::beast::http::string_body> req_post{boost::beast::http::verb::post,  // POST REQUEST
                                                                        "/api/entity",                   // URI
                                                                        11};                             // HTTP 1.1
  req_post.body() = req_data;

  auto status_code = handler.HandleRequest(req_post, res);
  ASSERT_EQ(status_code, INTERNAL_SERVER_ERROR);
}

TEST_F(CrudHandlerTest, CrudFailedCreateFile) {
  // set up Crud handler for post
  std::string file_path = "/crud/entity";

  EXPECT_CALL(manager, CreateDir(testing::_)).WillOnce(testing::Return(true)).WillOnce(testing::Return(true));
  EXPECT_CALL(manager, CreateFile(testing::_, testing::_)).WillOnce(testing::Return(false));

  CrudHandler handler(file_path, file_to_id, &manager);

  // set up post request
  string req_data = "{json}";
  boost::beast::http::request<boost::beast::http::string_body> req_post{boost::beast::http::verb::post,  // POST REQUEST
                                                                        "/api/entity",                   // URI
                                                                        11};                             // HTTP 1.1
  req_post.body() = req_data;

  auto status_code = handler.HandleRequest(req_post, res);
  ASSERT_EQ(status_code, INTERNAL_SERVER_ERROR);
}

TEST_F(CrudHandlerTest, CrudFailedWriteFile) {
  file_to_id["/crud/entity"].insert(1);

  // set up Crud handler for put
  std::string file_path = "/crud/entity/1";

  EXPECT_CALL(manager, CreateDir(testing::_)).WillOnce(testing::Return(true));
  EXPECT_CALL(manager, WriteFile(testing::_, testing::_)).WillOnce(testing::Return(false));

  CrudHandler handler(file_path, file_to_id, &manager);

  // set up put request
  string req_data = "{json}";
  boost::beast::http::request<boost::beast::http::string_body> req_put{boost::beast::http::verb::put,  // POST REQUEST
                                                                       "/api/entity/1",                // URI
                                                                       11};                            // HTTP 1.1
  req_put.body() = req_data;

  auto status_code = handler.HandleRequest(req_put, res);
  ASSERT_EQ(status_code, INTERNAL_SERVER_ERROR);
}

TEST_F(CrudHandlerTest, CrudFailedDeleteFile) {
  file_to_id["/crud/entity"].insert(1);

  // set up Crud handler
  std::string file_path = "/crud/entity/1";

  EXPECT_CALL(manager, CreateDir(testing::_)).WillOnce(testing::Return(true));
  EXPECT_CALL(manager, DeleteFile(testing::_)).WillOnce(testing::Return(false));

  CrudHandler handler(file_path, file_to_id, &manager);

  // set up request
  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::delete_,  // DELETE REQUEST
                                                                   "/api/entity/1",                    // URI
                                                                   11};                                // HTTP 1.1

  auto status_code = handler.HandleRequest(req, res);
  ASSERT_EQ(status_code, INTERNAL_SERVER_ERROR);
}

TEST_F(CrudHandlerTest, CrudFailedReadFile) {
  file_to_id["/crud/entity"].insert(1);

  // set up Crud handler
  std::string file_path = "/crud/entity/1";

  EXPECT_CALL(manager, CreateDir(testing::_)).WillOnce(testing::Return(true));
  boost::beast::http::response<boost::beast::http::dynamic_body> test_res;
  boost::beast::ostream(test_res.body()) << "{json}\n\n";
  EXPECT_CALL(manager, ReadFile(testing::_, testing::_))
      .WillOnce(testing::DoAll(testing::SetArgReferee<1>(test_res), testing::Return(false)));

  CrudHandler handler(file_path, file_to_id, &manager);

  // set up request
  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get,  // GET REQUEST
                                                                   "/api/entity/1",                // URI
                                                                   11};                            // HTTP 1.1

  auto status_code = handler.HandleRequest(req, res);
  ASSERT_EQ(status_code, INTERNAL_SERVER_ERROR);
}

TEST_F(CrudHandlerTest, CrudFailedListFiles) {
  file_to_id["/crud/entity"].insert(1);
  file_to_id["/crud/entity"].insert(2);

  // set up Crud handler
  std::string file_path = "/crud/entity";
  CrudHandler handler(file_path, file_to_id, &manager);

  EXPECT_CALL(manager, CreateDir(testing::_)).WillOnce(testing::Return(true));
  std::vector<std::string> file_names{"1", "2"};
  EXPECT_CALL(manager, ListFiles(testing::_, testing::_))
      .WillOnce(testing::DoAll(testing::SetArgReferee<1>(file_names), testing::Return(false)));

  // set up request
  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get,  // GET REQUEST
                                                                   "/api/entity",                  // URI
                                                                   11};                            // HTTP 1.1
  auto status_code = handler.HandleRequest(req, res);
  ASSERT_EQ(status_code, INTERNAL_SERVER_ERROR);
}

TEST_F(CrudHandlerTest, CrudInitRootFailedCreateDir) {
  boost::beast::http::request<boost::beast::http::string_body> req{
      boost::beast::http::verb::connect,  // Unsupported method
      "/api/entity/1",                    // URI
      11};                                // HTTP 1.1
  std::string file_path = "/crud/entity/1";

  EXPECT_CALL(manager, CreateDir(testing::_)).WillOnce(testing::Return(false));

  CrudHandler handler(file_path, file_to_id, &manager);
  auto status_code = handler.HandleRequest(req, res);
  ASSERT_EQ(status_code, INTERNAL_SERVER_ERROR);
}