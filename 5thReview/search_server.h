#pragma once

#include "document.h"
#include "string_processing.h"
#include "concurrent_map.h"
#include "log_duration.h"

#include <execution>
#include <map>
#include <set>
#include <future>
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_set>

const int MAX_RESULT_DOCUMENT_COUNT = 5;

template <typename ExecutionPolicy, typename ForwardRange, typename Function>
void ForEach(const ExecutionPolicy& policy, ForwardRange& range, Function function) {
    if constexpr (
            std::is_same_v<ExecutionPolicy, std::execution::sequenced_policy>
            || std::is_same_v<typename std::iterator_traits<typename ForwardRange::iterator>::iterator_category,
                    std::random_access_iterator_tag>
            ) {
        for_each(policy, range.begin(), range.end(), function);

    } else {
        static constexpr int PART_COUNT = 4;
        const auto part_length = size(range) / PART_COUNT;
        auto part_begin = range.begin();
        auto part_end = next(part_begin, part_length);

        std::vector<std::future<void>> futures;
        for (int i = 0;
             i < PART_COUNT;
             ++i,
                     part_begin = part_end,
                     part_end = (i == PART_COUNT - 1
                                 ? range.end()
                                 : next(part_begin, part_length))
                ) {
            futures.push_back(std::async([function, part_begin, part_end] {
                for_each(part_begin, part_end, function);
            }));
        }
    }
}

template <typename ForwardRange, typename Function>
void ForEach(ForwardRange& range, Function function) {
    ForEach(std::execution::seq, range, function);
}

class SearchServer {
public:

    //объявление конструкторов SearchServer
    explicit SearchServer(const std::string& stop_words_text);
    explicit SearchServer(std::string_view stop_words_text);

    //реализация шаблонного конструктора SearchServer
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words)
            : stop_words_(MakeUniqueNonEmptyStrings(stop_words))  // Extract non-empty stop words
    {
        if (!all_of(stop_words_.begin(), stop_words_.end(), IsValidWord)) {
            throw std::invalid_argument("Some of stop words are invalid"s);
        }
    }
    //объявление метода AddDocument
    void AddDocument(int document_id, std::string_view document, DocumentStatus status, const std::vector<int>& ratings);

    //объявление методов RemoveDocument
    void RemoveDocument(int document_id);
    void RemoveDocument(const std::execution::sequenced_policy&, int document_id);
    void RemoveDocument(const std::execution::parallel_policy&, int document_id);

    //объявление методов FindTopDocuments
    [[nodiscard]] std::vector<Document> FindTopDocuments(std::string_view raw_query, DocumentStatus status) const;
    [[nodiscard]] std::vector<Document> FindTopDocuments(std::string_view raw_query) const;

    //реализация шаблонных методов FindTopDocument
    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(std::string_view raw_query, DocumentPredicate document_predicate) const {
        return FindTopDocuments(std::execution::seq, raw_query, document_predicate);
    }
    template <typename ExecutionPolicy, typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const ExecutionPolicy exec_policy, std::string_view raw_query, DocumentPredicate document_predicate) const {
        //LOG_DURATION("FindTopDocuments");
        Query query = ParseQuery(raw_query);
        std::vector<Document> matched_documents = FindAllDocuments(exec_policy, query, document_predicate);
        sort(matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
            if (std::abs(lhs.relevance - rhs.relevance) < 1e-6) {
                return lhs.rating > rhs.rating;
            }
            else {
                return lhs.relevance > rhs.relevance;
            }
        });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }
    template <typename ExecutionPolicy>
    std::vector<Document> FindTopDocuments(const  ExecutionPolicy exec_policy, std::string_view raw_query) const {
        return FindTopDocuments(exec_policy, raw_query, DocumentStatus::ACTUAL);
    }
    template <typename ExecutionPolicy>
    std::vector<Document> FindTopDocuments(const  ExecutionPolicy exec_policy, std::string_view raw_query, DocumentStatus status) const {
        return FindTopDocuments(exec_policy, raw_query, [status](int document_id, DocumentStatus statusp, int rating) { return statusp == status; });
    }

    [[nodiscard]] int GetDocumentCount() const;

    [[nodiscard]] const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const;

    [[nodiscard]] std::set<int>::const_iterator begin() const;
    [[nodiscard]] std::set<int>::const_iterator end() const;

    //объявление методов MatchDocument
    [[nodiscard]] std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(std::string_view raw_query, int document_id) const;
    [[nodiscard]] std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::execution::sequenced_policy&, std::string_view raw_query, int document_id) const;
    [[nodiscard]] std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::execution::parallel_policy&, std::string_view raw_query, int document_id) const;

private:
    struct DocumentData {
        int rating = {};
        DocumentStatus status = {};
    };

    std::map<std::string, std::pair<std::string, std::string_view>> words_in_docs_;
    const std::set<std::string, std::less<>> stop_words_;
    std::map<std::string_view, std::map<int, double>> word_to_document_freqs_;
    std::map<int, DocumentData> documents_;
    std::set<int> document_ids_;
    std::map<int, std::map<std::string_view, double>> document_words_freqs_;

    [[nodiscard]] bool IsStopWord(std::string_view word) const;
    static bool IsValidWord(std::string_view word);

    [[nodiscard]] std::vector<std::string_view> SplitIntoWordsNoStop(std::string_view text) const;
    static int ComputeAverageRating(const std::vector<int>& ratings);

    struct QueryWord {
        std::string_view data;
        bool is_minus;
        bool is_stop;
    };

    [[nodiscard]] QueryWord ParseQueryWord(std::string_view text) const;

    struct Query {
        std::set<std::string_view> plus_words;
        std::set<std::string_view> minus_words;
    };

    [[nodiscard]] Query ParseQuery(const std::string_view& text) const;

    [[nodiscard]] double ComputeWordInverseDocumentFreq(std::string_view word) const;

    //реализация приватного шаблонного метода FindAllDocuments
    template <typename ExecutionPolicy, typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const ExecutionPolicy exec_policy, const Query& query, DocumentPredicate document_predicate) const {
        ConcurrentMap<int, double> document_to_relevance((query.plus_words).size());
        std::set<int> stop_ids;
        std::mutex m;
        ForEach(exec_policy,
                query.minus_words,
                [this, &stop_ids, &m](std::string_view word) {
                    if (word_to_document_freqs_.count(word)) {
                        for (const auto[document_id, _] : word_to_document_freqs_.at(word)) {
                            std::lock_guard guard(m);
                            stop_ids.insert(document_id);
                        }
                    }
                });

        ForEach(
                exec_policy,
                query.plus_words,
                [this, document_predicate, &document_to_relevance, &stop_ids](std::string_view word) {
                    if (word_to_document_freqs_.count(word)) {
                        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);

                        for (const auto[document_id, term_freq] : word_to_document_freqs_.at(word)) {
                            const auto &document_data = documents_.at(document_id);
                            if (document_predicate(document_id, document_data.status, document_data.rating) &&
                                (stop_ids.count(document_id) == 0)) {
                                document_to_relevance[document_id].ref_to_value +=
                                        term_freq * inverse_document_freq;
                            }
                        }
                    }
                });
        std::vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance.BuildOrdinaryMap()) {
            matched_documents.push_back({ document_id, relevance, documents_.at(document_id).rating });
        }

        return matched_documents;
    }
};