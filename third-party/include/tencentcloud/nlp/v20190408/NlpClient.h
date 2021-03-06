/*
 * Copyright (c) 2017-2019 THL A29 Limited, a Tencent company. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TENCENTCLOUD_NLP_V20190408_NLPCLIENT_H_
#define TENCENTCLOUD_NLP_V20190408_NLPCLIENT_H_

#include <functional>
#include <future>
#include <tencentcloud/core/AbstractClient.h>
#include <tencentcloud/core/Credential.h>
#include <tencentcloud/core/profile/ClientProfile.h>
#include <tencentcloud/core/AsyncCallerContext.h>
#include <tencentcloud/nlp/v20190408/model/AutoSummarizationRequest.h>
#include <tencentcloud/nlp/v20190408/model/AutoSummarizationResponse.h>
#include <tencentcloud/nlp/v20190408/model/ChatBotRequest.h>
#include <tencentcloud/nlp/v20190408/model/ChatBotResponse.h>
#include <tencentcloud/nlp/v20190408/model/CreateDictRequest.h>
#include <tencentcloud/nlp/v20190408/model/CreateDictResponse.h>
#include <tencentcloud/nlp/v20190408/model/CreateWordItemsRequest.h>
#include <tencentcloud/nlp/v20190408/model/CreateWordItemsResponse.h>
#include <tencentcloud/nlp/v20190408/model/DeleteDictRequest.h>
#include <tencentcloud/nlp/v20190408/model/DeleteDictResponse.h>
#include <tencentcloud/nlp/v20190408/model/DeleteWordItemsRequest.h>
#include <tencentcloud/nlp/v20190408/model/DeleteWordItemsResponse.h>
#include <tencentcloud/nlp/v20190408/model/DependencyParsingRequest.h>
#include <tencentcloud/nlp/v20190408/model/DependencyParsingResponse.h>
#include <tencentcloud/nlp/v20190408/model/DescribeDictRequest.h>
#include <tencentcloud/nlp/v20190408/model/DescribeDictResponse.h>
#include <tencentcloud/nlp/v20190408/model/DescribeDictsRequest.h>
#include <tencentcloud/nlp/v20190408/model/DescribeDictsResponse.h>
#include <tencentcloud/nlp/v20190408/model/DescribeWordItemsRequest.h>
#include <tencentcloud/nlp/v20190408/model/DescribeWordItemsResponse.h>
#include <tencentcloud/nlp/v20190408/model/KeywordsExtractionRequest.h>
#include <tencentcloud/nlp/v20190408/model/KeywordsExtractionResponse.h>
#include <tencentcloud/nlp/v20190408/model/LexicalAnalysisRequest.h>
#include <tencentcloud/nlp/v20190408/model/LexicalAnalysisResponse.h>
#include <tencentcloud/nlp/v20190408/model/SearchWordItemsRequest.h>
#include <tencentcloud/nlp/v20190408/model/SearchWordItemsResponse.h>
#include <tencentcloud/nlp/v20190408/model/SentenceEmbeddingRequest.h>
#include <tencentcloud/nlp/v20190408/model/SentenceEmbeddingResponse.h>
#include <tencentcloud/nlp/v20190408/model/SentimentAnalysisRequest.h>
#include <tencentcloud/nlp/v20190408/model/SentimentAnalysisResponse.h>
#include <tencentcloud/nlp/v20190408/model/SimilarWordsRequest.h>
#include <tencentcloud/nlp/v20190408/model/SimilarWordsResponse.h>
#include <tencentcloud/nlp/v20190408/model/TextClassificationRequest.h>
#include <tencentcloud/nlp/v20190408/model/TextClassificationResponse.h>
#include <tencentcloud/nlp/v20190408/model/TextCorrectionRequest.h>
#include <tencentcloud/nlp/v20190408/model/TextCorrectionResponse.h>
#include <tencentcloud/nlp/v20190408/model/TextCorrectionProRequest.h>
#include <tencentcloud/nlp/v20190408/model/TextCorrectionProResponse.h>
#include <tencentcloud/nlp/v20190408/model/TextSimilarityRequest.h>
#include <tencentcloud/nlp/v20190408/model/TextSimilarityResponse.h>
#include <tencentcloud/nlp/v20190408/model/TextSimilarityProRequest.h>
#include <tencentcloud/nlp/v20190408/model/TextSimilarityProResponse.h>
#include <tencentcloud/nlp/v20190408/model/UpdateDictRequest.h>
#include <tencentcloud/nlp/v20190408/model/UpdateDictResponse.h>
#include <tencentcloud/nlp/v20190408/model/WordEmbeddingRequest.h>
#include <tencentcloud/nlp/v20190408/model/WordEmbeddingResponse.h>
#include <tencentcloud/nlp/v20190408/model/WordSimilarityRequest.h>
#include <tencentcloud/nlp/v20190408/model/WordSimilarityResponse.h>


namespace TencentCloud
{
    namespace Nlp
    {
        namespace V20190408
        {
            class NlpClient : public AbstractClient
            {
            public:
                NlpClient(const Credential &credential, const std::string &region);
                NlpClient(const Credential &credential, const std::string &region, const ClientProfile &profile);

                typedef Outcome<Core::Error, Model::AutoSummarizationResponse> AutoSummarizationOutcome;
                typedef std::future<AutoSummarizationOutcome> AutoSummarizationOutcomeCallable;
                typedef std::function<void(const NlpClient*, const Model::AutoSummarizationRequest&, AutoSummarizationOutcome, const std::shared_ptr<const AsyncCallerContext>&)> AutoSummarizationAsyncHandler;
                typedef Outcome<Core::Error, Model::ChatBotResponse> ChatBotOutcome;
                typedef std::future<ChatBotOutcome> ChatBotOutcomeCallable;
                typedef std::function<void(const NlpClient*, const Model::ChatBotRequest&, ChatBotOutcome, const std::shared_ptr<const AsyncCallerContext>&)> ChatBotAsyncHandler;
                typedef Outcome<Core::Error, Model::CreateDictResponse> CreateDictOutcome;
                typedef std::future<CreateDictOutcome> CreateDictOutcomeCallable;
                typedef std::function<void(const NlpClient*, const Model::CreateDictRequest&, CreateDictOutcome, const std::shared_ptr<const AsyncCallerContext>&)> CreateDictAsyncHandler;
                typedef Outcome<Core::Error, Model::CreateWordItemsResponse> CreateWordItemsOutcome;
                typedef std::future<CreateWordItemsOutcome> CreateWordItemsOutcomeCallable;
                typedef std::function<void(const NlpClient*, const Model::CreateWordItemsRequest&, CreateWordItemsOutcome, const std::shared_ptr<const AsyncCallerContext>&)> CreateWordItemsAsyncHandler;
                typedef Outcome<Core::Error, Model::DeleteDictResponse> DeleteDictOutcome;
                typedef std::future<DeleteDictOutcome> DeleteDictOutcomeCallable;
                typedef std::function<void(const NlpClient*, const Model::DeleteDictRequest&, DeleteDictOutcome, const std::shared_ptr<const AsyncCallerContext>&)> DeleteDictAsyncHandler;
                typedef Outcome<Core::Error, Model::DeleteWordItemsResponse> DeleteWordItemsOutcome;
                typedef std::future<DeleteWordItemsOutcome> DeleteWordItemsOutcomeCallable;
                typedef std::function<void(const NlpClient*, const Model::DeleteWordItemsRequest&, DeleteWordItemsOutcome, const std::shared_ptr<const AsyncCallerContext>&)> DeleteWordItemsAsyncHandler;
                typedef Outcome<Core::Error, Model::DependencyParsingResponse> DependencyParsingOutcome;
                typedef std::future<DependencyParsingOutcome> DependencyParsingOutcomeCallable;
                typedef std::function<void(const NlpClient*, const Model::DependencyParsingRequest&, DependencyParsingOutcome, const std::shared_ptr<const AsyncCallerContext>&)> DependencyParsingAsyncHandler;
                typedef Outcome<Core::Error, Model::DescribeDictResponse> DescribeDictOutcome;
                typedef std::future<DescribeDictOutcome> DescribeDictOutcomeCallable;
                typedef std::function<void(const NlpClient*, const Model::DescribeDictRequest&, DescribeDictOutcome, const std::shared_ptr<const AsyncCallerContext>&)> DescribeDictAsyncHandler;
                typedef Outcome<Core::Error, Model::DescribeDictsResponse> DescribeDictsOutcome;
                typedef std::future<DescribeDictsOutcome> DescribeDictsOutcomeCallable;
                typedef std::function<void(const NlpClient*, const Model::DescribeDictsRequest&, DescribeDictsOutcome, const std::shared_ptr<const AsyncCallerContext>&)> DescribeDictsAsyncHandler;
                typedef Outcome<Core::Error, Model::DescribeWordItemsResponse> DescribeWordItemsOutcome;
                typedef std::future<DescribeWordItemsOutcome> DescribeWordItemsOutcomeCallable;
                typedef std::function<void(const NlpClient*, const Model::DescribeWordItemsRequest&, DescribeWordItemsOutcome, const std::shared_ptr<const AsyncCallerContext>&)> DescribeWordItemsAsyncHandler;
                typedef Outcome<Core::Error, Model::KeywordsExtractionResponse> KeywordsExtractionOutcome;
                typedef std::future<KeywordsExtractionOutcome> KeywordsExtractionOutcomeCallable;
                typedef std::function<void(const NlpClient*, const Model::KeywordsExtractionRequest&, KeywordsExtractionOutcome, const std::shared_ptr<const AsyncCallerContext>&)> KeywordsExtractionAsyncHandler;
                typedef Outcome<Core::Error, Model::LexicalAnalysisResponse> LexicalAnalysisOutcome;
                typedef std::future<LexicalAnalysisOutcome> LexicalAnalysisOutcomeCallable;
                typedef std::function<void(const NlpClient*, const Model::LexicalAnalysisRequest&, LexicalAnalysisOutcome, const std::shared_ptr<const AsyncCallerContext>&)> LexicalAnalysisAsyncHandler;
                typedef Outcome<Core::Error, Model::SearchWordItemsResponse> SearchWordItemsOutcome;
                typedef std::future<SearchWordItemsOutcome> SearchWordItemsOutcomeCallable;
                typedef std::function<void(const NlpClient*, const Model::SearchWordItemsRequest&, SearchWordItemsOutcome, const std::shared_ptr<const AsyncCallerContext>&)> SearchWordItemsAsyncHandler;
                typedef Outcome<Core::Error, Model::SentenceEmbeddingResponse> SentenceEmbeddingOutcome;
                typedef std::future<SentenceEmbeddingOutcome> SentenceEmbeddingOutcomeCallable;
                typedef std::function<void(const NlpClient*, const Model::SentenceEmbeddingRequest&, SentenceEmbeddingOutcome, const std::shared_ptr<const AsyncCallerContext>&)> SentenceEmbeddingAsyncHandler;
                typedef Outcome<Core::Error, Model::SentimentAnalysisResponse> SentimentAnalysisOutcome;
                typedef std::future<SentimentAnalysisOutcome> SentimentAnalysisOutcomeCallable;
                typedef std::function<void(const NlpClient*, const Model::SentimentAnalysisRequest&, SentimentAnalysisOutcome, const std::shared_ptr<const AsyncCallerContext>&)> SentimentAnalysisAsyncHandler;
                typedef Outcome<Core::Error, Model::SimilarWordsResponse> SimilarWordsOutcome;
                typedef std::future<SimilarWordsOutcome> SimilarWordsOutcomeCallable;
                typedef std::function<void(const NlpClient*, const Model::SimilarWordsRequest&, SimilarWordsOutcome, const std::shared_ptr<const AsyncCallerContext>&)> SimilarWordsAsyncHandler;
                typedef Outcome<Core::Error, Model::TextClassificationResponse> TextClassificationOutcome;
                typedef std::future<TextClassificationOutcome> TextClassificationOutcomeCallable;
                typedef std::function<void(const NlpClient*, const Model::TextClassificationRequest&, TextClassificationOutcome, const std::shared_ptr<const AsyncCallerContext>&)> TextClassificationAsyncHandler;
                typedef Outcome<Core::Error, Model::TextCorrectionResponse> TextCorrectionOutcome;
                typedef std::future<TextCorrectionOutcome> TextCorrectionOutcomeCallable;
                typedef std::function<void(const NlpClient*, const Model::TextCorrectionRequest&, TextCorrectionOutcome, const std::shared_ptr<const AsyncCallerContext>&)> TextCorrectionAsyncHandler;
                typedef Outcome<Core::Error, Model::TextCorrectionProResponse> TextCorrectionProOutcome;
                typedef std::future<TextCorrectionProOutcome> TextCorrectionProOutcomeCallable;
                typedef std::function<void(const NlpClient*, const Model::TextCorrectionProRequest&, TextCorrectionProOutcome, const std::shared_ptr<const AsyncCallerContext>&)> TextCorrectionProAsyncHandler;
                typedef Outcome<Core::Error, Model::TextSimilarityResponse> TextSimilarityOutcome;
                typedef std::future<TextSimilarityOutcome> TextSimilarityOutcomeCallable;
                typedef std::function<void(const NlpClient*, const Model::TextSimilarityRequest&, TextSimilarityOutcome, const std::shared_ptr<const AsyncCallerContext>&)> TextSimilarityAsyncHandler;
                typedef Outcome<Core::Error, Model::TextSimilarityProResponse> TextSimilarityProOutcome;
                typedef std::future<TextSimilarityProOutcome> TextSimilarityProOutcomeCallable;
                typedef std::function<void(const NlpClient*, const Model::TextSimilarityProRequest&, TextSimilarityProOutcome, const std::shared_ptr<const AsyncCallerContext>&)> TextSimilarityProAsyncHandler;
                typedef Outcome<Core::Error, Model::UpdateDictResponse> UpdateDictOutcome;
                typedef std::future<UpdateDictOutcome> UpdateDictOutcomeCallable;
                typedef std::function<void(const NlpClient*, const Model::UpdateDictRequest&, UpdateDictOutcome, const std::shared_ptr<const AsyncCallerContext>&)> UpdateDictAsyncHandler;
                typedef Outcome<Core::Error, Model::WordEmbeddingResponse> WordEmbeddingOutcome;
                typedef std::future<WordEmbeddingOutcome> WordEmbeddingOutcomeCallable;
                typedef std::function<void(const NlpClient*, const Model::WordEmbeddingRequest&, WordEmbeddingOutcome, const std::shared_ptr<const AsyncCallerContext>&)> WordEmbeddingAsyncHandler;
                typedef Outcome<Core::Error, Model::WordSimilarityResponse> WordSimilarityOutcome;
                typedef std::future<WordSimilarityOutcome> WordSimilarityOutcomeCallable;
                typedef std::function<void(const NlpClient*, const Model::WordSimilarityRequest&, WordSimilarityOutcome, const std::shared_ptr<const AsyncCallerContext>&)> WordSimilarityAsyncHandler;



                /**
                 *???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
                 * @param req AutoSummarizationRequest
                 * @return AutoSummarizationOutcome
                 */
                AutoSummarizationOutcome AutoSummarization(const Model::AutoSummarizationRequest &request);
                void AutoSummarizationAsync(const Model::AutoSummarizationRequest& request, const AutoSummarizationAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context = nullptr);
                AutoSummarizationOutcomeCallable AutoSummarizationCallable(const Model::AutoSummarizationRequest& request);

                /**
                 *?????????????????????????????????NLP??????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????


                 * @param req ChatBotRequest
                 * @return ChatBotOutcome
                 */
                ChatBotOutcome ChatBot(const Model::ChatBotRequest &request);
                void ChatBotAsync(const Model::ChatBotRequest& request, const ChatBotAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context = nullptr);
                ChatBotOutcomeCallable ChatBotCallable(const Model::ChatBotRequest& request);

                /**
                 *??????????????????????????????????????????????????????
                 * @param req CreateDictRequest
                 * @return CreateDictOutcome
                 */
                CreateDictOutcome CreateDict(const Model::CreateDictRequest &request);
                void CreateDictAsync(const Model::CreateDictRequest& request, const CreateDictAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context = nullptr);
                CreateDictOutcomeCallable CreateDictCallable(const Model::CreateDictRequest& request);

                /**
                 *????????????????????????????????????
                 * @param req CreateWordItemsRequest
                 * @return CreateWordItemsOutcome
                 */
                CreateWordItemsOutcome CreateWordItems(const Model::CreateWordItemsRequest &request);
                void CreateWordItemsAsync(const Model::CreateWordItemsRequest& request, const CreateWordItemsAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context = nullptr);
                CreateWordItemsOutcomeCallable CreateWordItemsCallable(const Model::CreateWordItemsRequest& request);

                /**
                 *???????????????????????????????????????????????????????????????????????????
                 * @param req DeleteDictRequest
                 * @return DeleteDictOutcome
                 */
                DeleteDictOutcome DeleteDict(const Model::DeleteDictRequest &request);
                void DeleteDictAsync(const Model::DeleteDictRequest& request, const DeleteDictAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context = nullptr);
                DeleteDictOutcomeCallable DeleteDictCallable(const Model::DeleteDictRequest& request);

                /**
                 *??????????????????????????????????????????
                 * @param req DeleteWordItemsRequest
                 * @return DeleteWordItemsOutcome
                 */
                DeleteWordItemsOutcome DeleteWordItems(const Model::DeleteWordItemsRequest &request);
                void DeleteWordItemsAsync(const Model::DeleteWordItemsRequest& request, const DeleteWordItemsAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context = nullptr);
                DeleteWordItemsOutcomeCallable DeleteWordItemsCallable(const Model::DeleteWordItemsRequest& request);

                /**
                 *??????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
                 * @param req DependencyParsingRequest
                 * @return DependencyParsingOutcome
                 */
                DependencyParsingOutcome DependencyParsing(const Model::DependencyParsingRequest &request);
                void DependencyParsingAsync(const Model::DependencyParsingRequest& request, const DependencyParsingAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context = nullptr);
                DependencyParsingOutcomeCallable DependencyParsingCallable(const Model::DependencyParsingRequest& request);

                /**
                 *??????id???????????????????????????????????????
                 * @param req DescribeDictRequest
                 * @return DescribeDictOutcome
                 */
                DescribeDictOutcome DescribeDict(const Model::DescribeDictRequest &request);
                void DescribeDictAsync(const Model::DescribeDictRequest& request, const DescribeDictAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context = nullptr);
                DescribeDictOutcomeCallable DescribeDictCallable(const Model::DescribeDictRequest& request);

                /**
                 *?????????????????????????????????????????????????????????
                 * @param req DescribeDictsRequest
                 * @return DescribeDictsOutcome
                 */
                DescribeDictsOutcome DescribeDicts(const Model::DescribeDictsRequest &request);
                void DescribeDictsAsync(const Model::DescribeDictsRequest& request, const DescribeDictsAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context = nullptr);
                DescribeDictsOutcomeCallable DescribeDictsCallable(const Model::DescribeDictsRequest& request);

                /**
                 *????????????????????????ID?????????????????????????????????
                 * @param req DescribeWordItemsRequest
                 * @return DescribeWordItemsOutcome
                 */
                DescribeWordItemsOutcome DescribeWordItems(const Model::DescribeWordItemsRequest &request);
                void DescribeWordItemsAsync(const Model::DescribeWordItemsRequest& request, const DescribeWordItemsAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context = nullptr);
                DescribeWordItemsOutcomeCallable DescribeWordItemsCallable(const Model::DescribeWordItemsRequest& request);

                /**
                 *????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
                 * @param req KeywordsExtractionRequest
                 * @return KeywordsExtractionOutcome
                 */
                KeywordsExtractionOutcome KeywordsExtraction(const Model::KeywordsExtractionRequest &request);
                void KeywordsExtractionAsync(const Model::KeywordsExtractionRequest& request, const KeywordsExtractionAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context = nullptr);
                KeywordsExtractionOutcomeCallable KeywordsExtractionCallable(const Model::KeywordsExtractionRequest& request);

                /**
                 *?????????????????????????????????????????????

1???????????????????????????????????????????????????????????????????????????????????????????????????????????????

2?????????????????????????????????????????????????????????????????????????????????????????????????????????

3????????????????????????????????????????????????????????????????????????????????????????????????

????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
                 * @param req LexicalAnalysisRequest
                 * @return LexicalAnalysisOutcome
                 */
                LexicalAnalysisOutcome LexicalAnalysis(const Model::LexicalAnalysisRequest &request);
                void LexicalAnalysisAsync(const Model::LexicalAnalysisRequest& request, const LexicalAnalysisAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context = nullptr);
                LexicalAnalysisOutcomeCallable LexicalAnalysisCallable(const Model::LexicalAnalysisRequest& request);

                /**
                 *??????????????????????????????????????????????????????
                 * @param req SearchWordItemsRequest
                 * @return SearchWordItemsOutcome
                 */
                SearchWordItemsOutcome SearchWordItems(const Model::SearchWordItemsRequest &request);
                void SearchWordItemsAsync(const Model::SearchWordItemsRequest& request, const SearchWordItemsAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context = nullptr);
                SearchWordItemsOutcomeCallable SearchWordItemsCallable(const Model::SearchWordItemsRequest& request);

                /**
                 *??????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????

?????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????Bert?????????????????????????????????????????????????????????????????????????????????NLP??????????????????????????????
                 * @param req SentenceEmbeddingRequest
                 * @return SentenceEmbeddingOutcome
                 */
                SentenceEmbeddingOutcome SentenceEmbedding(const Model::SentenceEmbeddingRequest &request);
                void SentenceEmbeddingAsync(const Model::SentenceEmbeddingRequest& request, const SentenceEmbeddingAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context = nullptr);
                SentenceEmbeddingOutcomeCallable SentenceEmbeddingCallable(const Model::SentenceEmbeddingRequest& request);

                /**
                 *?????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????

???????????????????????????????????????????????????LSTM???BERT????????????????????????????????????????????????????????????????????????????????????????????????
                 * @param req SentimentAnalysisRequest
                 * @return SentimentAnalysisOutcome
                 */
                SentimentAnalysisOutcome SentimentAnalysis(const Model::SentimentAnalysisRequest &request);
                void SentimentAnalysisAsync(const Model::SentimentAnalysisRequest& request, const SentimentAnalysisAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context = nullptr);
                SentimentAnalysisOutcomeCallable SentimentAnalysisCallable(const Model::SentimentAnalysisRequest& request);

                /**
                 *???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
                 * @param req SimilarWordsRequest
                 * @return SimilarWordsOutcome
                 */
                SimilarWordsOutcome SimilarWords(const Model::SimilarWordsRequest &request);
                void SimilarWordsAsync(const Model::SimilarWordsRequest& request, const SimilarWordsAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context = nullptr);
                SimilarWordsOutcomeCallable SimilarWordsCallable(const Model::SimilarWordsRequest& request);

                /**
                 *?????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????

???????????????????????????????????????????????????LSTM???BERT????????????????????????????????????????????????????????????????????????????????????????????????

??????????????????

- ????????????????????????????????????????????????14???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
                 * @param req TextClassificationRequest
                 * @return TextClassificationOutcome
                 */
                TextClassificationOutcome TextClassification(const Model::TextClassificationRequest &request);
                void TextClassificationAsync(const Model::TextClassificationRequest& request, const TextClassificationAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context = nullptr);
                TextClassificationOutcomeCallable TextClassificationCallable(const Model::TextClassificationRequest& request);

                /**
                 *????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????2000????????????????????????????????????????????????

??????????????????????????????????????????????????????LSTM???BERT???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????? 
                 * @param req TextCorrectionRequest
                 * @return TextCorrectionOutcome
                 */
                TextCorrectionOutcome TextCorrection(const Model::TextCorrectionRequest &request);
                void TextCorrectionAsync(const Model::TextCorrectionRequest& request, const TextCorrectionAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context = nullptr);
                TextCorrectionOutcomeCallable TextCorrectionCallable(const Model::TextCorrectionRequest& request);

                /**
                 *????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????128????????????????????????????????????????????????

??????????????????????????????????????????????????????LSTM???BERT????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
                 * @param req TextCorrectionProRequest
                 * @return TextCorrectionProOutcome
                 */
                TextCorrectionProOutcome TextCorrectionPro(const Model::TextCorrectionProRequest &request);
                void TextCorrectionProAsync(const Model::TextCorrectionProRequest& request, const TextCorrectionProAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context = nullptr);
                TextCorrectionProOutcomeCallable TextCorrectionProCallable(const Model::TextCorrectionProRequest& request);

                /**
                 *???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????500???????????????????????????????????????????????????????????????????????????

????????????????????????????????????????????????????????????????????????????????????????????????Bert??????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????

??????????????????????????????????????????????????????????????????????????????????????????
                 * @param req TextSimilarityRequest
                 * @return TextSimilarityOutcome
                 */
                TextSimilarityOutcome TextSimilarity(const Model::TextSimilarityRequest &request);
                void TextSimilarityAsync(const Model::TextSimilarityRequest& request, const TextSimilarityAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context = nullptr);
                TextSimilarityOutcomeCallable TextSimilarityCallable(const Model::TextSimilarityRequest& request);

                /**
                 *???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????128???????????????????????????????????????????????????????????????????????????

????????????????????????????????????????????????????????????????????????????????????????????????Bert??????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????

??????????????????????????????????????????????????????????????????????????????????????????
                 * @param req TextSimilarityProRequest
                 * @return TextSimilarityProOutcome
                 */
                TextSimilarityProOutcome TextSimilarityPro(const Model::TextSimilarityProRequest &request);
                void TextSimilarityProAsync(const Model::TextSimilarityProRequest& request, const TextSimilarityProAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context = nullptr);
                TextSimilarityProOutcomeCallable TextSimilarityProCallable(const Model::TextSimilarityProRequest& request);

                /**
                 *???????????????????????????????????????????????????????????????
                 * @param req UpdateDictRequest
                 * @return UpdateDictOutcome
                 */
                UpdateDictOutcome UpdateDict(const Model::UpdateDictRequest &request);
                void UpdateDictAsync(const Model::UpdateDictRequest& request, const UpdateDictAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context = nullptr);
                UpdateDictOutcomeCallable UpdateDictCallable(const Model::UpdateDictRequest& request);

                /**
                 *?????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????

?????????????????????????????????????????????????????????????????????AI Lab?????????????????????????????????????????????????????????????????????????????????AI Lab?????????DSG?????????????????????????????????????????????800????????????????????????????????????????????????????????????????????????????????????

??????AI Lab????????????????????????

https://ai.tencent.com/ailab/zh/news/detial?id=22

https://ai.tencent.com/ailab/nlp/zh/embedding.html
                 * @param req WordEmbeddingRequest
                 * @return WordEmbeddingOutcome
                 */
                WordEmbeddingOutcome WordEmbedding(const Model::WordEmbeddingRequest &request);
                void WordEmbeddingAsync(const Model::WordEmbeddingRequest& request, const WordEmbeddingAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context = nullptr);
                WordEmbeddingOutcomeCallable WordEmbeddingCallable(const Model::WordEmbeddingRequest& request);

                /**
                 *?????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
                 * @param req WordSimilarityRequest
                 * @return WordSimilarityOutcome
                 */
                WordSimilarityOutcome WordSimilarity(const Model::WordSimilarityRequest &request);
                void WordSimilarityAsync(const Model::WordSimilarityRequest& request, const WordSimilarityAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context = nullptr);
                WordSimilarityOutcomeCallable WordSimilarityCallable(const Model::WordSimilarityRequest& request);

            };
        }
    }
}

#endif // !TENCENTCLOUD_NLP_V20190408_NLPCLIENT_H_
