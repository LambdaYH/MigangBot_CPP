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
#include "service/service.h"

namespace white {

class Trie {
 public:
  Trie();
  ~Trie();

 public:
  bool Insert(const std::string &key, std::shared_ptr<Service> service);

  bool InsertFromBack(const std::string &key, std::shared_ptr<Service> service);

  const SearchResult Search(const std::string &key) const noexcept;

  const SearchResult SearchFromBack(const std::string &key) const noexcept;

 private:
  struct TrieNode {
    std::unordered_map<char, std::shared_ptr<TrieNode>> childs;
    std::shared_ptr<Service> service;
  };

 private:
  std::shared_ptr<TrieNode> root_;
  std::shared_ptr<Service> no_service_here_;
};

inline Trie::Trie() : root_(std::make_shared<TrieNode>()), no_service_here_() {}

inline Trie::~Trie() {}

inline bool Trie::Insert(const std::string &key, std::shared_ptr<Service> service) {
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

inline bool Trie::InsertFromBack(const std::string &key, std::shared_ptr<Service> service) {
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

inline const SearchResult Trie::Search(const std::string &key) const noexcept {
  auto cur_node = root_;
  short command_size{0};
  for (auto ch : key) {
    ch = std::tolower(ch);
    if (ch == ' ') break;
    ++command_size;
    if (!cur_node->childs.count(ch)) return {no_service_here_, 0};
    cur_node = cur_node->childs[ch];
    if (cur_node->service) return {cur_node->service, command_size};
  }
  if (cur_node->service) return {cur_node->service, command_size};
  return {no_service_here_, 0};
}

inline const SearchResult Trie::SearchFromBack(
    const std::string &key) const noexcept {
  auto cur_node = root_;
  short command_size{0};
  for (auto it = key.rbegin(); it != key.rend(); ++it) {
    auto ch = std::tolower(*it);
    if (ch == ' ') break;
    --command_size;
    if (!cur_node->childs.count(ch)) return {no_service_here_, 0};
    cur_node = cur_node->childs[ch];
    if (cur_node->service) return {cur_node->service, command_size};
  }
  if (cur_node->service) return {cur_node->service, command_size};
  return {no_service_here_, 0};
}

}  // namespace white

#endif