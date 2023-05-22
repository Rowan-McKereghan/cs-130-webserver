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
  MOCK_METHOD(bool, ReadFile, (boost::filesystem::path path,  boost::beast::http::response<boost::beast::http::dynamic_body>& res), (override));
  MOCK_METHOD(bool, ListFiles, (boost::filesystem::path path, std::vector<std::string>& out_vect), (override));
};


TEST(CrudHandler, GetNextIDNewEntity) {
  std::string file_path = "/crud";
  unordered_map<std::string, std::unordered_set<int>> file_to_id;
  MockManager manager;

  CrudHandler handler(file_path, file_to_id, &manager);

  int res = handler.GetNextID("entity");
  EXPECT_EQ(res, 1);
}

TEST(CrudHandler, GetNextIDNormal) {
  std::string file_path = "/crud";
  unordered_map<std::string, std::unordered_set<int>> file_to_id;
  file_to_id["entity"].insert(1);
  file_to_id["entity"].insert(2);
  MockManager manager;

  CrudHandler handler(file_path, file_to_id, &manager);

  int res = handler.GetNextID("entity");
  EXPECT_EQ(res, 3);
}

TEST(CrudHandler, GetNextIDDeletedNum) {
  std::string file_path = "/crud";
  unordered_map<std::string, std::unordered_set<int>> file_to_id;
  file_to_id["entity"].insert(1);
  file_to_id["entity"].insert(3);
  MockManager manager;

  CrudHandler handler(file_path, file_to_id, &manager);

  int res = handler.GetNextID("entity");
  EXPECT_EQ(res, 2);
}

TEST(CrudHandler, ParseTargetNoID) {
  std::string file_path = "/crud/entity";
  unordered_map<std::string, std::unordered_set<int>> file_to_id;
  MockManager manager;

  CrudHandler handler(file_path, file_to_id, &manager);

  std::pair<std::string, int> res = handler.ParseTarget();
  EXPECT_EQ(res.first, "/crud/entity");
  EXPECT_EQ(res.second, -1);
}

TEST(CrudHandler, ParseTargetYesID) {
  std::string file_path = "/crud/entity/1";
  unordered_map<std::string, std::unordered_set<int>> file_to_id;
  MockManager manager;

  CrudHandler handler(file_path, file_to_id, &manager);

  std::pair<std::string, int> res = handler.ParseTarget();
  EXPECT_EQ(res.first, "/crud/entity");
  EXPECT_EQ(res.second, 1);
}

// tests HandleRequest with an unsupported HTTP method
TEST(CrudHandler, CrudTestUnsuportedMethod) {
  string req_data = "Irrelevant stuff in request body";
  boost::beast::http::request<boost::beast::http::string_body> req{
      boost::beast::http::verb::connect,  // Unsupported method
      "/api/entity/1",                    // URI
      11};                                // HTTP 1.1
  req.body() = req_data;
  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  std::string file_path = "/crud/entity/1";
  unordered_map<std::string, std::unordered_set<int>> file_to_id;
  MockManager manager;
  EXPECT_CALL(manager, CreateDir(testing::_)).WillOnce(testing::Return(true));

  CrudHandler handler(file_path, file_to_id, &manager);
  handler.HandleRequest(req, res);

  EXPECT_EQ(res.version(), 11);                                                      // version
  EXPECT_EQ(res.result_int(), BAD_REQUEST);                                          // status
  EXPECT_EQ(boost::beast::buffers_to_string(res.body().data()), "400 Bad Request\n\n");  // body
}

// tests HandleRequest using Post method
TEST(CrudHandler, CrudTestPost) {
  //set up map
  unordered_map<std::string, std::unordered_set<int>> file_to_id;
  
  // set up Crud handler for post
  std::string file_path = "/crud/entity";
  MockManager manager;
  EXPECT_CALL(manager, CreateDir(testing::_)).WillOnce(testing::Return(true)).WillOnce(testing::Return(true));
  EXPECT_CALL(manager, CreateFile(testing::_, testing::_)).WillOnce(testing::Return(true));

  CrudHandler handler(file_path, file_to_id, &manager);
  
  // set up post request
  string req_data = "{json}";
  boost::beast::http::request<boost::beast::http::string_body> req_post{
      boost::beast::http::verb::post,  // POST REQUEST
      "/api/entity",                    // URI
      11};                                // HTTP 1.1
  req_post.body() = req_data;
  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  handler.HandleRequest(req_post, res);

  
  // check post response
  EXPECT_EQ(res.version(), 11);                                                      // version
  EXPECT_EQ(res.result_int(), OK);                                          // status
  EXPECT_EQ(boost::beast::buffers_to_string(res.body().data()), "{\"id\": 1}\n\n");  // body
}

// tests HandleRequest using Put method
TEST(CrudHandler, CrudTestPut) {
  //set up map
  unordered_map<std::string, std::unordered_set<int>> file_to_id;
  //set up map
  file_to_id["/crud/entity"].insert(1);

  // set up Crud handler for put
  std::string file_path = "/crud/entity/1";
  MockManager manager;
  EXPECT_CALL(manager, WriteFile(testing::_, testing::_)).WillOnce(testing::Return(true));

  CrudHandler handler(file_path, file_to_id, &manager);
  
  // set up put request
  string req_data = "{json}";
  boost::beast::http::request<boost::beast::http::string_body> req_put{
      boost::beast::http::verb::put,  // PUT REQUEST
      "/api/entity/1",                    // URI
      11};                                // HTTP 1.1
  req_put.body() = req_data;
  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  handler.HandleRequest(req_put, res);

  
  // check put response
  EXPECT_EQ(res.version(), 11);                                                      // version
  EXPECT_EQ(res.result_int(), OK);                                          // status
  EXPECT_EQ(boost::beast::buffers_to_string(res.body().data()), "{\"id\": 1}\n\n");  // body
}

// tests HandleRequest using Delete method
TEST(CrudHandler, CrudTestDelete) {
  //set up map
  unordered_map<std::string, std::unordered_set<int>> file_to_id;
  //set up map
  file_to_id["/crud/entity"].insert(1);

  // set up Crud handler
  std::string file_path = "/crud/entity/1";
  MockManager manager;
  EXPECT_CALL(manager, DeleteFile(testing::_)).WillOnce(testing::Return(true));

  CrudHandler handler(file_path, file_to_id, &manager);
  
  // set up put request
  boost::beast::http::request<boost::beast::http::string_body> req{
      boost::beast::http::verb::delete_,  // DELETE REQUEST
      "/api/entity/1",                    // URI
      11};                                // HTTP 1.1
  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  handler.HandleRequest(req, res);

  
  // check put response
  EXPECT_EQ(res.version(), 11);                                                      // version
  EXPECT_EQ(res.result_int(), OK);                                          // status
  EXPECT_EQ(boost::beast::buffers_to_string(res.body().data()), "{\"id\": 1}\n\n");  // body

  //check that id was removed from set
  bool wasRemoved = file_to_id["/crud/entity"].find(1) == file_to_id["/crud/entity"].end();
  EXPECT_TRUE(wasRemoved);
}

// tests HandleRequest using Get method using ID
TEST(CrudHandler, CrudTestGetID) {
  //set up map
  unordered_map<std::string, std::unordered_set<int>> file_to_id;
  //set up map
  file_to_id["/crud/entity"].insert(1);

  // set up Crud handler
  std::string file_path = "/crud/entity/1";
  MockManager manager;
  boost::beast::http::response<boost::beast::http::dynamic_body> test_res;
  boost::beast::ostream(test_res.body()) << "{json}\n\n";
  EXPECT_CALL(manager, ReadFile(testing::_, testing::_)).WillOnce(DoAll(testing::SetArgReferee<1>(test_res), testing::Return(true)));

  CrudHandler handler(file_path, file_to_id, &manager);
  
  // set up put request
  boost::beast::http::request<boost::beast::http::string_body> req{
      boost::beast::http::verb::get,  // GET REQUEST
      "/api/entity/1",                    // URI
      11};                                // HTTP 1.1
  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  handler.HandleRequest(req, res);

  
  // check put response
  EXPECT_EQ(res.version(), 11);                                                      // version
  EXPECT_EQ(res.result_int(), OK);                                          // status
  EXPECT_EQ(boost::beast::buffers_to_string(res.body().data()), "{json}\n\n");  // body
}

// tests HandleRequest using Get method without ID to list files
TEST(CrudHandler, CrudTestGetNoID) {
  //set up map
  unordered_map<std::string, std::unordered_set<int>> file_to_id;
  //set up map
  file_to_id["/crud/entity"].insert(1);
  file_to_id["/crud/entity"].insert(2);

  // set up Crud handler
  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  std::string file_path = "/crud/entity";
  MockManager manager;
  std::vector<std::string> file_names{"1", "2"};
  EXPECT_CALL(manager, ListFiles(testing::_, testing::_)).WillOnce(DoAll(testing::SetArgReferee<1>(file_names), testing::Return(true)));

  CrudHandler handler(file_path, file_to_id, &manager);
  
  // set up request
  boost::beast::http::request<boost::beast::http::string_body> req{
      boost::beast::http::verb::get,  // GET REQUEST
      "/api/entity",                    // URI
      11};                                // HTTP 1.1
  handler.HandleRequest(req, res);

  
  // check put response
  EXPECT_EQ(res.version(), 11);                                                      // version
  EXPECT_EQ(res.result_int(), OK);                                          // status
  EXPECT_EQ(boost::beast::buffers_to_string(res.body().data()), "[1, 2]\n\n");  // body
}


// tests HandleRequest using Get method with an entity that does not exist
TEST(CrudHandler, CrudTestGetNonexistantEntity) {
  //set up map
  unordered_map<std::string, std::unordered_set<int>> file_to_id;

  // set up Crud handler
  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  std::string file_path = "/crud/entity";
  MockManager manager;

  CrudHandler handler(file_path, file_to_id, &manager);
  
  // set up request
  boost::beast::http::request<boost::beast::http::string_body> req{
      boost::beast::http::verb::get,  // GET REQUEST
      "/api/entity",                    // URI
      11};                                // HTTP 1.1
  handler.HandleRequest(req, res);

  
  // check put response
  EXPECT_EQ(res.version(), 11);                                                      // version
  EXPECT_EQ(res.result_int(), OK);                                          // status
  EXPECT_EQ(boost::beast::buffers_to_string(res.body().data()), "[]\n\n");  // body
}