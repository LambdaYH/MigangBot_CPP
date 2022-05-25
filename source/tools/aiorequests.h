#ifndef MIGANGBOT_TOOLS_AIOREQUESTS_H_
#define MIGANGBOT_TOOLS_AIOREQUESTS_H_

#include <hv/requests.h>

#include <future>
#include <iostream>
#include <memory>
#include <string>

#include "co_future.h"

namespace white {
namespace aiorequests {

using requests::Request;
using requests::Response;
using requests::ResponseCallback;

inline co_future<Response> aiorequest(Request &&req) {
  static hv::HttpClient cli;
  auto promise = std::make_shared<co_promise<Response>>();
  auto ret = promise->get_future();
  cli.sendAsync(req, [promise = std::move(promise)](const Response& resp) {
    if (resp == NULL)
      promise->set_value(Response());
    else
      promise->set_value(std::move(resp));
  });
  return ret;
}

inline co_future<Response> aiorequest(http_method method, const char* url,
                           const std::size_t timeout = 30,
                           const http_body& body = NoBody,
                           const http_headers& headers = DefaultHeaders) {
  auto req = std::make_shared<HttpRequest>();
  req->method = method;
  req->url = url;
  req->timeout = timeout;
  if (&body != &NoBody) {
    req->body = body;
  }
  if (&headers != &DefaultHeaders) {
    req->headers = headers;
  }
  return aiorequest(std::move(req));
}

inline co_future<Response> Get(const std::string& url, std::size_t timeout = 30,
                    const http_headers& headers = DefaultHeaders) {
  return aiorequest(HTTP_GET, url.c_str(), timeout, NoBody, headers);
}

inline co_future<Response> Post(const std::string& url, std::size_t timeout = 30,
                     const http_body& body = NoBody,
                     const http_headers& headers = DefaultHeaders) {
  return aiorequest(HTTP_POST, url.c_str(), timeout, body, headers);
}

}  // namespace aiorequests
}  // namespace white

#endif