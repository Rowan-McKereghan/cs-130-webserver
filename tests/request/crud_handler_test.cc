#include "crud_handler.h"

#include <boost/beast/core.hpp>

#include "crud_filesystem_manager.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class MockManager : public CrudFileSystemManager {
 public:
  MockManager() : CrudFileSystemManager(){};
  MOCK_METHOD(bool, CreateDir, (boost::filesystem::path), (override));
};

TEST(CrudHandler, GetNextIDNewEntity) {
  std::string file_path = "/crud";
  unordered_map<std::string, std::list<int>> file_to_id;
  MockManager manager;

  CrudHandler handler(file_path, file_to_id, &manager);

  int res = handler.GetNextID("entity");
  EXPECT_EQ(res, 1);
}

TEST(CrudHandler, GetNextIDNormal) {
  std::string file_path = "/crud";
  unordered_map<std::string, std::list<int>> file_to_id;
  file_to_id["entity"].push_back(1);
  file_to_id["entity"].push_back(2);
  MockManager manager;

  CrudHandler handler(file_path, file_to_id, &manager);

  int res = handler.GetNextID("entity");
  EXPECT_EQ(res, 3);
}

TEST(CrudHandler, GetNextIDDeletedNum) {
  std::string file_path = "/crud";
  unordered_map<std::string, std::list<int>> file_to_id;
  file_to_id["entity"].push_back(1);
  file_to_id["entity"].push_back(3);
  MockManager manager;

  CrudHandler handler(file_path, file_to_id, &manager);

  int res = handler.GetNextID("entity");
  EXPECT_EQ(res, 2);
}

TEST(CrudHandler, ParseTargetNoID) {
  std::string file_path = "/crud/entity";
  unordered_map<std::string, std::list<int>> file_to_id;
  MockManager manager;

  CrudHandler handler(file_path, file_to_id, &manager);

  std::pair<std::string, int> res = handler.ParseTarget();
  EXPECT_EQ(res.first, "/crud/entity");
  EXPECT_EQ(res.second, -1);
}

TEST(CrudHandler, ParseTargetYesID) {
  std::string file_path = "/crud/entity/1";
  unordered_map<std::string, std::list<int>> file_to_id;
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
  unordered_map<std::string, std::list<int>> file_to_id;
  MockManager manager;
  EXPECT_CALL(manager, CreateDir(testing::_)).WillOnce(testing::Return(true));

  CrudHandler handler(file_path, file_to_id, &manager);
  handler.HandleRequest(req, res);

  EXPECT_EQ(res.version(), 11);                                                      // version
  EXPECT_EQ(res.result_int(), BAD_REQUEST);                                          // status
  EXPECT_EQ(boost::beast::buffers_to_string(res.body().data()), "400 Bad Request");  // body
}