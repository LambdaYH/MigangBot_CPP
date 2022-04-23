#include "hv/WebSocketClient.h"
#include "nlohmann/json.hpp"
#include <thread>
#include <vector>
#include <algorithm>

using namespace hv;

nlohmann::json set_echo(const nlohmann::json &t)
{
    nlohmann::json ret_message{
        {"data", {
            {"message_id", 1023027117}
        }},
        {"echo", t["echo"]},
        {"retcode", 0},
        {"status", "ok"}
    };
    return ret_message;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: %s url count thread_num\n", argv[0]);
        return -10;
    }
    const char* url = argv[1];
    const std::size_t loop_count = atoi(argv[2]);
    const std::size_t thread_num = atoi(argv[3]);
    std::vector<std::size_t> count_per_thread(thread_num);

    nlohmann::json message{
        {"anonymous", nullptr},
        {"font", 0},
        {"group_id",855134081},
        {"message", "[CQ:at,qq=2418650100] /e asdasdad"},
        {"message_id", -817611999},
        {"message_seq", 177238},
        {"message_type", "group"},
        {"post_type","message"},
        {"raw_message", "[CQ:at,qq=2418650100] /e asdasdad"},
        {"self_id", 2418650100},
        {"sender", {}},
        {"sub_type", "normal"},
        {"time", 1650633329},
        {"user_id", 1214847481}
    };

    std::string msg_str = message.dump();


    std::vector<std::thread> threads;
    for(std::size_t i = 0; i < thread_num; ++i)
    {
        threads.push_back(std::thread{
            [loop_count = loop_count, &msg_str, &url, i = i, &count_per_thread]()
            {
                WebSocketClient ws;
                ws.onopen = []() {
                    printf("onopen\n");
                };
                ws.onclose = []() {
                    printf("onclose\n");
                };
                ws.onmessage = [&](const std::string& msg) {
                    ++count_per_thread[i];
                    nlohmann::json json_msg = nlohmann::json::parse(msg);
                    ws.send(set_echo(json_msg).dump());
                    std::cout << "Recv: " << msg <<std::endl;
                };

                reconn_setting_t reconn;
                reconn_setting_init(&reconn);
                reconn.min_delay = 1;
                reconn.max_delay = 10;
                reconn.delay_policy = 2;
                ws.setReconnect(&reconn);

                http_headers headers;
                ws.open(url, headers);
                
                for(std::size_t i = 0; i < loop_count; ++i)
                {
                    if(!ws.isConnected())
                        continue;
                    ws.send(msg_str);
                }
                while(ws.isConnected()) {}
            }
        });
    }
    for(auto &t : threads)
        t.join();
    std::cout << "Total Send Count: " << loop_count * thread_num << std::endl;
    std::cout << "Success: " << std::accumulate(count_per_thread.begin(), count_per_thread.end(), 0) << std::endl;

    return 0;
}