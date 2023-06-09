#include "global_websocket_state.h"

#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "logger.h"
#include "websocket_handler.h"

GlobalWebsocketState::GlobalWebsocketState() {}

void GlobalWebsocketState::Add(const std::string& channel, WebsocketHandler* session) {
  std::unique_lock<std::shared_mutex> map_lock(map_mutex_);
  if (map_.find(std::string(channel)) == map_.end()) {
    LOG(info) << "Creating channel: " << channel;
    map_.emplace(channel,
                 std::make_pair(std::make_unique<std::shared_mutex>(), std::unordered_set<WebsocketHandler*>{}));
  }
  auto& channel_ = map_.at(channel);
  map_lock.unlock();

  std::unique_lock<std::shared_mutex> channel_lock(*channel_.first);
  channel_.second.insert(session);
}

void GlobalWebsocketState::Remove(const std::string& channel, WebsocketHandler* session) {
  std::unique_lock<std::shared_mutex> map_lock(map_mutex_);
  auto& channel_ = map_.at(channel);
  map_lock.unlock();

  std::unique_lock<std::shared_mutex> channel_lock(*channel_.first);
  LOG(info) << "Removing from: " << channel << ". There are now: " << channel_.second.size() << " sessions.";
  channel_.second.erase(session);
}

void GlobalWebsocketState::Broadcast(const std::string& channel, const boost::beast::flat_buffer& message) {
  std::shared_ptr<boost::beast::flat_buffer const> const& message_ =
      std::make_shared<boost::beast::flat_buffer const>(std::move(message));
  std::unique_lock<std::shared_mutex> map_lock(map_mutex_);

  bool broadcast_channels = false;
  std::string message_str = boost::beast::buffers_to_string(message.data());
  if (message_str.find("userName") != std::string::npos && message_str.find("text") != std::string::npos) {
    broadcast_channels = true;
  }

  std::string response = "{ \"available_channels\": [";

  auto& channel_ = map_.at(channel);
  for (auto it = map_.begin(); it != map_.end(); it++) {
    std::string chan_name = it->first;
    chan_name.erase(0, 1);
    response += "\"" + chan_name + "\", ";
  }

  // Don't need to lock the map after we're done traversing through it
  map_lock.unlock();

  if (response.length() > 1) {
    response.erase(response.length() - 2, 2);
  }
  response += "]}\n\n";
  boost::beast::flat_buffer response_cast;
  boost::beast::ostream(response_cast) << response;

  std::shared_ptr<boost::beast::flat_buffer const> const& response_cast_pointer =
      std::make_shared<boost::beast::flat_buffer const>(std::move(response_cast));

  std::vector<std::weak_ptr<WebsocketHandler>> v;
  {
    std::unique_lock<std::shared_mutex> channel_lock(*channel_.first);
    v.reserve(channel_.second.size());
    for (auto p : channel_.second) {
      v.emplace_back(p->weak_from_this());
    }
  }

  // For each session in our local list, try to acquire a strong
  // pointer. If successful, then send the message on that session.
  for (auto const& wp : v) {
    if (auto sp = wp.lock()) {
      std::string channel_name = sp->GetChannel();
      sp->Broadcast(message_);
      if (broadcast_channels) {
        sp->Broadcast(response_cast_pointer);
      }
    }
  }

  for (auto const& wp : v) {
    if (auto sp = wp.lock()) {
    }
  }
}