#ifndef MIGANGBOT_MODULE_SERVER_H_
#define MIGANGBOT_MODULE_SERVER_H_

#include <hv/WebSocketServer.h>

#include <set>
#include <string>

#include "bot/bot.h"
#include "logger/logger.h"

namespace white {

class Server {
 public:
  Server(const unsigned short port, const std::string& addr = "0.0.0.0") {
    InitWebsocketService();
    InitHttpService();
    server_.port = port;
    strcpy(server_.host, addr.c_str());
    server_.service = &http_;
    server_.ws = &ws_;
  }

  void Run() {
    websocket_server_run(&server_, 0);

    while (getchar() != '\n')
      ;
    websocket_server_stop(&server_);
  }

 private:
  void InitWebsocketService() {
    ws_.onopen = [this](const WebSocketChannelPtr& channel,
                        const std::string& url) {
      LOG_DEBUG("onopen: GET {}", url);
      Bot* bot = channel->newContext<Bot>();
      go(&Bot::Run, bot, channel);
    };
    ws_.onmessage = [](const WebSocketChannelPtr& channel,
                       const std::string& msg) {
      LOG_DEBUG("Get Message: {}", msg);
      go(&Bot::OnRead, channel->getContext<Bot>(), msg);
    };
    ws_.onclose = [this](const WebSocketChannelPtr& channel) {
      LOG_DEBUG("onClose");
      channel->deleteContext<Bot>();
    };
  }

  void InitHttpService() {
    http_.GET("/ping",
              [](const HttpContextPtr& ctx) { return ctx->send("pong"); });
  }

 private:
  hv::WebSocketService ws_;
  hv::HttpService http_;

  websocket_server_t server_;

};

}  // namespace white

#endif