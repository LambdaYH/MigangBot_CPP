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

inline Response aiorequest(Request req) {
  hv::HttpClient cli;
  co_promise<Response> promise;
  cli.sendAsync(req, [&promise](const Response& resp) {
    if (resp == NULL)
      promise.set_value(Response());
    else
      promise.set_value(resp);
  });
  return promise.get_future().get();
}

inline Response aiorequest(http_method method, const char* url,
                           const std::size_t timeout = 30,
                           const http_body& body = NoBody,
                           const http_headers& headers = DefaultHeaders) {
  Request req(new HttpRequest);
  req->method = method;
  req->url = url;
  req->timeout = timeout;
  if (&body != &NoBody) {
    req->body = body;
  }
  if (&headers != &DefaultHeaders) {
    req->headers = headers;
  }
  return aiorequest(req);
}

inline Response Get(const std::string& url, std::size_t timeout = 30,
                    const http_headers& headers = DefaultHeaders) {
  return aiorequest(HTTP_GET, url.c_str(), timeout, NoBody, headers);
}

inline Response Post(const std::string& url, std::size_t timeout = 30,
                     const http_body& body = NoBody,
                     const http_headers& headers = DefaultHeaders) {
  return aiorequest(HTTP_POST, url.c_str(), timeout, body, headers);
}

}  // namespace aiorequests
}  // namespace white

#endif