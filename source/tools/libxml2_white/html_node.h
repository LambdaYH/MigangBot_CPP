#ifndef MIGANGBOT_TOOLS_LIBXML2_WHITE_HTML_NODE_H_
#define MIGANGBOT_TOOLS_LIBXML2_WHITE_HTML_NODE_H_

#include <cstring>

#include <vector>
#include <string>
#include <stack>
#include "libxml/tree.h"

#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>

namespace white {
class HtmlNode {
 public:
  HtmlNode(htmlNodePtr node) : node_(node) {}
  HtmlNode Parent();
  HtmlNode Child();
  std::string Content();
  void SetContent(const std::string &content);
  std::vector<HtmlNode> FindChildrenByAttribute(const std::string &attr,
                                                const std::string &value,
                                                bool recursive = true);

  std::string GetAllText();

  std::string GetAtrribute(const std::string &attr);

  operator bool() const { return node_ != nullptr; }

 private:
  htmlNodePtr node_;
};

inline HtmlNode HtmlNode::Parent() { return HtmlNode(node_->parent); }

inline HtmlNode HtmlNode::Child() { return HtmlNode(node_->children); }

inline void TextNodeDfs(htmlNodePtr node, std::string &ret) {
  for (auto cur_node = node; cur_node; cur_node = cur_node->next) {
    if (cur_node->type == XML_TEXT_NODE) {
      ret += reinterpret_cast<char *>(cur_node->content);
    }
    TextNodeDfs(cur_node->children, ret);
  }
}

inline std::string HtmlNode::Content() {
  return reinterpret_cast<char *>(node_->content);
}

inline void HtmlNode::SetContent(const std::string &content) {
  auto c = xmlEncodeEntitiesReentrant(
      node_->doc,
      xmlEncodeEntitiesReentrant(
          node_->doc, reinterpret_cast<const xmlChar *>(content.c_str())));
  xmlNodeSetContent(node_, c);
}

inline std::string HtmlNode::GetAllText() {
  if (!node_) return "";
  std::string ret;
  TextNodeDfs(node_, ret);
  return ret;
}

inline std::string HtmlNode::GetAtrribute(const std::string &attr) {
  if (!node_ ||
      !xmlHasProp(node_, reinterpret_cast<const xmlChar *>(attr.c_str())))
    return "";
  return reinterpret_cast<const char *>(
      xmlGetProp(node_, reinterpret_cast<const xmlChar *>(attr.c_str())));
}

inline std::vector<HtmlNode> HtmlNode::FindChildrenByAttribute(
    const std::string &attr, const std::string &value, bool recursive) {
  if (!node_ || !node_->children) return {};
  std::vector<HtmlNode> ret;
  std::stack<htmlNodePtr> stk;
  stk.push(node_->children);
  auto attr_xmlchar = reinterpret_cast<const xmlChar *>(attr.c_str());
  auto value_c_str = value.c_str();
  while (!stk.empty()) {
    auto p = stk.top();
    stk.pop();
    for (auto cur_node = p; cur_node; cur_node = cur_node->next) {
      if (cur_node->type == XML_ELEMENT_NODE) {
        if (xmlHasProp(cur_node, attr_xmlchar) &&
            strcasecmp((const char *)xmlGetProp(cur_node, attr_xmlchar),
                       value_c_str) == 0)
          ret.push_back(cur_node);
      }
      if (recursive) stk.push(cur_node->children);
    }
  }
  return ret;
}

}  // namespace white

#endif