#ifndef MIGANGBOTCPP_MODULES_MODULE_TENCENTCLOUD_NLP_AUTO_SUMMARIZATION_H_
#define MIGANGBOTCPP_MODULES_MODULE_TENCENTCLOUD_NLP_AUTO_SUMMARIZATION_H_

#include "modules/module_interface.h"
#include <functional>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <string_view>
#include <tencentcloud/core/Credential.h>
#include <tencentcloud/core/profile/ClientProfile.h>
#include <tencentcloud/core/profile/HttpProfile.h>
#include <tencentcloud/nlp/v20190408/NlpClient.h>
#include <tencentcloud/nlp/v20190408/model/AutoSummarizationRequest.h>
#include <tencentcloud/nlp/v20190408/model/AutoSummarizationResponse.h>

#include "modules/module/tencentcloud_nlp/config.h"

namespace white
{
namespace module
{

class AutoSummarization : public Module
{
public:
    AutoSummarization() : Module("tencentcloud_nlp/api_key.yml", tencentcloud::kConfigExample) {
        auto config = LoadConfig();
        secret_id_ = config["SECRET_ID"].as<std::string>();
        secret_key_ = config["SECRET_KEY"].as<std::string>();
    }
    virtual void Register();

    void SummarizationExtraction(const Event &event, onebot11::ApiBot &bot);

private:
    std::string GetSummarization(const std::string &text);

private:
    std::string secret_id_;
    std::string secret_key_;
};

inline void AutoSummarization::Register()
{
    RegisterCommand(SUFFIX, {"摘要提取", "/摘要提取"}, func(AutoSummarization::SummarizationExtraction));
}

inline void AutoSummarization::SummarizationExtraction(const Event &event, onebot11::ApiBot &bot)
{
    auto msg = ExtraPlainText(event);
    auto text = std::string_view(msg);
    bot.send_msg(event, GetSummarization(std::string(text)));
}

inline std::string AutoSummarization::GetSummarization(const std::string &text)
{
    using namespace TencentCloud;
    using namespace TencentCloud::Nlp::V20190408;
    using namespace TencentCloud::Nlp::V20190408::Model;
    Credential cred = Credential(secret_id_, secret_key_);

    HttpProfile httpProfile = HttpProfile();
    httpProfile.SetEndpoint("nlp.tencentcloudapi.com");

    ClientProfile clientProfile = ClientProfile();
    clientProfile.SetHttpProfile(httpProfile);
    NlpClient client = NlpClient(cred, "ap-guangzhou", clientProfile);

    AutoSummarizationRequest req = AutoSummarizationRequest();
    
    req.SetText(text);

    auto outcome = client.AutoSummarization(req);
    if (!outcome.IsSuccess())
    {
        LOG_WARN("AutoSummarization: {}", outcome.GetError().PrintAll());
        return "";
    }
    AutoSummarizationResponse resp = outcome.GetResult();
    nlohmann::json resp_json = nlohmann::json::parse(resp.ToJsonString());
    return fmt::format("提取的摘要内容如下\n====================\n{}", resp_json["Summary"].get<std::string>());
}

} // namespace module
} // namespace white

#endif