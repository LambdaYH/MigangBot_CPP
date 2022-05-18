#ifndef MIGANGBOT_TOOLS_LIBXML2_WHITE_HTML_DOC_H_
#define MIGANGBOT_TOOLS_LIBXML2_WHITE_HTML_DOC_H_

#include <string>
#include <vector>
#include <stack>

#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>

#include "tools/libxml2_white/html_node.h"

namespace white {

class HtmlDoc {
 public:
  HtmlDoc() : doc_(nullptr) {}
  ~HtmlDoc() { xmlFreeDoc(doc_); }

  bool LoadFromString(const std::string &html) {
    doc_ = htmlReadMemory(html.c_str(), html.size(), "noname.html", NULL, 0);
    if (!doc_) return false;
    return true;
  }

  HtmlNode GetRootElement() {
    return HtmlNode(xmlDocGetRootElement(doc_));
  }

  std::string GetAllText() {
    return GetRootElement().GetAllText();
  }

 private:
  htmlDocPtr doc_;
};

} // namespace white

#endif