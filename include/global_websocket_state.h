#ifndef SHARED_STATE_H
#define SHARED_STATE_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/smart_ptr.hpp>
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_set>

class WebsocketHandler;

class GlobalWebsocketState {
 public:
  explicit GlobalWebsocketState();

  virtual void Add(const std::string& channel, WebsocketHandler* session);
  virtual void Remove(const std::string& channel, WebsocketHandler* session);
  virtual void Broadcast(const std::string& channel, const std::string& message);
  virtual void Broadcast(const std::string& channel, const boost::beast::flat_buffer& message);
  virtual std::vector<std::string> get_channels();

 private:
  std::unordered_map<std::string, std::pair<std::unique_ptr<std::shared_mutex>, std::unordered_set<WebsocketHandler*>>>
      map_;
  std::shared_mutex map_mutex_;
};

#endif
