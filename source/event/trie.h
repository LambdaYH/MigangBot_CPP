#ifndef MIGANGBOT_EVENT_TRIE_H_
#define MIGANGBOT_EVENT_TRIE_H_

#include <algorithm>
#include <cctype>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>

#include <nlohmann/json.hpp>

#include "bot/onebot_11/api_bot.h"
#include "service/triggered_service.h"
#include "event/type.h"

namespace white {

class Trie {
 public:
  Trie();
  ~Trie();

 public:
  bool Insert(const std::string &key,
              std::shared_ptr<TriggeredService> service) noexcept;

  bool InsertFromBack(const std::string &key,
                      std::shared_ptr<TriggeredService> service) noexcept;

  const std::shared_ptr<TriggeredService> &ShortestPrefix(
      const std::string &key, Event &event) const noexcept;

  const std::shared_ptr<TriggeredService> &LongestPrefix(
      const std::string &key, Event &event) const noexcept;

  const std::shared_ptr<TriggeredService> &LongestSuffix(
      const std::string &key, Event &event) const noexcept;

  const std::shared_ptr<TriggeredService> &ShortestSuffix(
      const std::string &key, Event &event) const noexcept;

 private:
  struct TrieNode {
    std::unordered_map<char, std::unique_ptr<TrieNode>> childs_;
    std::shared_ptr<TriggeredService> service_;
    std::size_t command_size_;
  };

 private:
  template <bool Is_From_Back, typename It>
  bool InsertImpl(It &&start, It &&end,
                  std::shared_ptr<TriggeredService> &&service) noexcept;

  template <typename It>
  const std::shared_ptr<TriggeredService> &ShortestSearchImpl(
      It &&start, It &&end, Event &event) const noexcept;

  template <typename It>
  const std::shared_ptr<TriggeredService> &LongestSearchImpl(
      It &&start, It &&end, Event &event) const noexcept;

 private:
  const std::unique_ptr<TrieNode> root_;
  const std::shared_ptr<TriggeredService> empty_;
};

inline Trie::Trie() : root_(std::make_unique<TrieNode>()) {}

inline Trie::~Trie() {}

template <bool Is_From_Back, typename It>
inline bool Trie::InsertImpl(
    It &&start, It &&end,
    std::shared_ptr<TriggeredService> &&service) noexcept {
  auto cur_node = root_.get();
  auto command_size = end - start;
  char ch;
  while (start != end) {
    ch = std::tolower(*start);
    if (!cur_node->childs_.count(ch))
      cur_node->childs_.emplace(ch, std::make_unique<TrieNode>());
    cur_node = cur_node->childs_.at(ch).get();
    ++start;
  }
  if (cur_node->service_) return false;
  cur_node->service_ = std::move(service);
  if constexpr (Is_From_Back)
    cur_node->command_size_ = -command_size;
  else
    cur_node->command_size_ = command_size;
  return true;
}

inline bool Trie::Insert(const std::string &key,
                         std::shared_ptr<TriggeredService> service) noexcept {
  return InsertImpl<false>(key.begin(), key.end(), std::move(service));
}

inline bool Trie::InsertFromBack(
    const std::string &key,
    std::shared_ptr<TriggeredService> service) noexcept {
  return InsertImpl<true>(key.rbegin(), key.rend(), std::move(service));
}

template <typename It>
inline const std::shared_ptr<TriggeredService> &Trie::ShortestSearchImpl(
    It &&start, It &&end, Event &event) const noexcept {
  auto cur_node = root_.get();
  char ch;
  while (start != end) {
    ch = std::tolower(*start);
    if (!cur_node->childs_.count(ch)) return empty_;
    cur_node = cur_node->childs_.at(ch).get();
    if (cur_node->service_) {
      event["__command_size__"] = cur_node->command_size_;
      return cur_node->service_;
    }
    ++start;
  }
  return empty_;
}

inline const std::shared_ptr<TriggeredService> &Trie::ShortestPrefix(
    const std::string &key, Event &event) const noexcept {
  return ShortestSearchImpl(key.begin(), key.end(), event);
}

inline const std::shared_ptr<TriggeredService> &Trie::ShortestSuffix(
    const std::string &key, Event &event) const noexcept {
  return ShortestSearchImpl(key.rbegin(), key.rend(), event);
}

template <typename It>
inline const std::shared_ptr<TriggeredService> &Trie::LongestSearchImpl(
    It &&start, It &&end, Event &event) const noexcept {
  auto cur_node = root_.get();
  char ch;
  TrieNode *pre{nullptr};
  while (start != end) {
    ch = std::tolower(*start);
    if (!cur_node->childs_.count(ch)) break;
    cur_node = cur_node->childs_.at(ch).get();
    if (cur_node->service_) pre = cur_node;
    ++start;
  }
  if (pre) {
    event["__command_size__"] = pre->command_size_;
    return pre->service_;
  }
  return empty_;
}

inline const std::shared_ptr<TriggeredService> &Trie::LongestPrefix(
    const std::string &key, Event &event) const noexcept {
  return LongestSearchImpl(key.begin(), key.end(), event);
}

inline const std::shared_ptr<TriggeredService> &Trie::LongestSuffix(
    const std::string &key, Event &event) const noexcept {
  return LongestSearchImpl(key.rbegin(), key.rend(), event);
}

}  // namespace white

#endif