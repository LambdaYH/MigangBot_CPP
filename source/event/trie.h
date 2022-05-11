#ifndef MIGANGBOT_EVENT_TRIE_H_
#define MIGANGBOT_EVENT_TRIE_H_

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include <nlohmann/json.hpp>

#include "bot/onebot_11/api_bot.h"
#include "event/search_result.h"
#include "event/type.h"

namespace white {

class Trie {
 public:
  Trie();
  ~Trie();

 public:
  bool Insert(const std::string &key,
              std::shared_ptr<TriggeredService> service);

  bool InsertFromBack(const std::string &key,
                      std::shared_ptr<TriggeredService> service);

  const std::shared_ptr<TriggeredService> &Search(const std::string &key,
                                                  Event &event) const noexcept;

  const std::shared_ptr<TriggeredService> &SearchFromBack(
      const std::string &key, Event &event) const noexcept;

 private:
  struct TrieNode {
    std::unordered_map<char, std::shared_ptr<TrieNode>> childs;
    std::shared_ptr<TriggeredService> service;
  };

 private:
  std::shared_ptr<TrieNode> root_;
  const std::shared_ptr<TriggeredService> empty_;
};

inline Trie::Trie() : root_(std::make_shared<TrieNode>()) {}

inline Trie::~Trie() {}

inline bool Trie::Insert(const std::string &key,
                         std::shared_ptr<TriggeredService> service) {
  auto cur_node = root_;
  for (auto ch : key) {
    ch = std::tolower(ch);
    if (!cur_node->childs.count(ch))
      cur_node->childs.emplace(ch, std::make_shared<TrieNode>());
    cur_node = cur_node->childs[ch];
  }
  if (cur_node->service) return false;
  cur_node->service = service;
  return true;
}

inline bool Trie::InsertFromBack(const std::string &key,
                                 std::shared_ptr<TriggeredService> service) {
  auto cur_node = root_;
  for (auto it = key.rbegin(); it != key.rend(); ++it) {
    auto ch = std::tolower(*it);
    if (!cur_node->childs.count(ch))
      cur_node->childs.emplace(ch, std::make_shared<TrieNode>());
    cur_node = cur_node->childs[ch];
  }
  if (cur_node->service) return false;
  cur_node->service = service;
  return true;
}

inline const std::shared_ptr<TriggeredService> &Trie::Search(
    const std::string &key, Event &event) const noexcept {
  auto cur_node = root_;
  short command_size{0};
  for (auto ch : key) {
    ch = std::tolower(ch);
    if (ch == ' ') break;
    ++command_size;
    if (!cur_node->childs.count(ch)) return empty_;
    cur_node = cur_node->childs[ch];
    if (cur_node->service) {
      event["__command_size__"] = command_size;
      return cur_node->service;
    }
  }
  if (cur_node->service) {
    event["__command_size__"] = command_size;
    return cur_node->service;
  }
  return empty_;
}

inline const std::shared_ptr<TriggeredService> &Trie::SearchFromBack(
    const std::string &key, Event &event) const noexcept {
  auto cur_node = root_;
  short command_size{0};
  for (auto it = key.rbegin(); it != key.rend(); ++it) {
    auto ch = std::tolower(*it);
    if (ch == ' ') break;
    --command_size;
    if (!cur_node->childs.count(ch)) return empty_;
    cur_node = cur_node->childs[ch];
    if (cur_node->service) {
      event["__command_size__"] = command_size;
      return cur_node->service;
    }
  }
  if (cur_node->service) {
    event["__command_size__"] = command_size;
    return cur_node->service;
  }
  return empty_;
}

}  // namespace white

#endif