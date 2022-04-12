#ifndef MIGANGBOTCPP_EVENT_HANDLER_TRIE_H_
#define MIGANGBOTCPP_EVENT_HANDLER_TRIE_H_

#include <string>
#include <functional>
#include <memory>
#include <unordered_map>
#include <jsoncpp/json/json.h>

namespace white
{

class Trie
{
public:
    Trie();
    ~Trie();

public:
    template<typename F>
    bool Insert(const std::string &key, F &&func);

    const plugin_func &Search(const std::string &key) const;
private:
    struct TrieNode
    {
        std::unordered_map<char, std::shared_ptr<TrieNode>> childs;
        plugin_func func;
    };

private:
    std::shared_ptr<TrieNode> root_;
    plugin_func no_func_here_;
};

inline Trie::Trie() :
root_(std::make_shared<TrieNode>()),
no_func_here_()
{

}

inline Trie::~Trie()
{

}

template<typename F>
inline bool Trie::Insert(const std::string &key, F &&func)
{
    auto cur_node = root_;
    for(auto ch : key)
    {
        ch = std::tolower(ch);
        if(!cur_node->childs.count(ch))
            cur_node->childs.emplace(ch, std::make_shared<TrieNode>());
        cur_node = cur_node->childs[ch];
    }
    if(cur_node->func)
        return false;
    cur_node->func = std::forward<F>(func);
    return true;
}

inline const plugin_func &Trie::Search(const std::string &key) const
{
    auto cur_node = root_;
    for(auto ch : key)
    {
        ch = std::tolower(ch);
        if(ch == ' ')
            break;
        if (!cur_node->childs.count(ch))
            return no_func_here_;
        cur_node = cur_node->childs[ch];
    }
    if(cur_node->func)
        return cur_node->func;
    return no_func_here_;
}

} // namespace white

#endif