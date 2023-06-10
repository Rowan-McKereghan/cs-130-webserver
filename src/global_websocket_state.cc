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
  auto& channel_ = map_.at(channel);
  map_lock.unlock();

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
      sp->Broadcast(message_);
    }
  }
}

std::vector<std::string> GlobalWebsocketState::GetChannels() {
  std::unique_lock<std::shared_mutex> map_lock(map_mutex_);
  std::vector<std::string> channels;
  for (auto it = map_.begin(); it != map_.end(); it++) {
	std::string channel = it->first;
    if (channel.size() > 1) {
      channel.erase(0, 1);
    }
    channel = "\"" + channel + "\"";
    channels.push_back(channel);
  }
  return channels;
}