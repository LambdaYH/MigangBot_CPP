#ifndef MIGANGBOTCPP_MODULES_MODULE_TENCENTCLOUD_NLP_KEYWORDS_EXTRACTION_H_
#define MIGANGBOTCPP_MODULES_MODULE_TENCENTCLOUD_NLP_KEYWORDS_EXTRACTION_H_

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
#include <tencentcloud/nlp/v20190408/model/KeywordsExtractionRequest.h>
#include <tencentcloud/nlp/v20190408/model/KeywordsExtractionResponse.h>

#include "modules/module/tencentcloud_nlp/config.h"

namespace white
{
namespace module
{

class KeywordsExtraction : public Module
{
public:
    KeywordsExtraction() : Module("tencentcloud_nlp/api_key.yml", tencentcloud::kConfigExample) {
        auto config = LoadConfig();
        secret_id_ = config["SECRET_ID"].as<std::string>();
        secret_key_ = config["SECRET_KEY"].as<std::string>();
    }
    virtual void Register();

    void KeywordEX(const Event &event, onebot11::ApiBot &bot);

private:
    std::string GetKeywords(const std::string &text);

private:
    std::string secret_id_;
    std::string secret_key_;
};

inline void KeywordsExtraction::Register()
{
    RegisterCommand(PREFIX, {"关键词提取", "/关键词提取"}, func(KeywordsExtraction::KeywordEX));
}

inline void KeywordsExtraction::KeywordEX(const Event &event, onebot11::ApiBot &bot)
{
    auto text = ExtraPlainText(event["message"].get<std::string>());
    Strip(text);
    bot.send_msg(event, GetKeywords(std::string(text)));
}

inline std::string KeywordsExtraction::GetKeywords(const std::string &text)
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

    KeywordsExtractionRequest req = KeywordsExtractionRequest();

    req.SetText(std::string(text));

    auto outcome = client.KeywordsExtraction(req);
    if (!outcome.IsSuccess())
    {
        LOG_WARN("KeywordsExtraction: {}", outcome.GetError().PrintAll());
        return "";
    }
    KeywordsExtractionResponse resp = outcome.GetResult();
    nlohmann::json resp_json = nlohmann::json::parse(resp.ToJsonString());
    resp_json = resp_json["Keywords"];
    std::string ret = "提取的关键词如下\n====================\n";
    for (std::size_t i = 0; i < resp_json.size(); ++i)
    {
        nlohmann::json &word = resp_json[i];
        if (word["Score"].get<double>() > 0.7)
            ret += word["Word"].get<std::string>() + " ";
        else
            break;
    }
    ret.pop_back();
    return ret;
}

} // namespace module
} // namespace white

#endif