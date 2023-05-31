#include "global_websocket_state.h"

#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

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

void GlobalWebsocketState::Broadcast(const std::string& channel, std::string message) {
  std::shared_ptr<std::string const> const& message_ = std::make_shared<std::string const>(std::move(message));
  std::unique_lock<std::shared_mutex> map_lock(map_mutex_);
  auto& channel_ = map_.at(channel);
  map_lock.unlock();

  // std::unique_lock<std::shared_mutex> channel_lock(*channel_.first);
  // for (auto const& sp : channel_.second) {
  //   sp->Broadcast(message_);
  // }

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
