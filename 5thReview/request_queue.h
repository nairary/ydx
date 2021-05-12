#pragma once

#include "search_server.h"
#include <deque>

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server);
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
        const auto documents = search_server_.FindTopDocuments(raw_query, document_predicate);

        AddRequest(documents.empty());

        return documents;
    }

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
    std::vector<Document> AddFindRequest(const std::string& raw_query);
    int GetNoResultRequests() const;
private:
    void AddRequest (bool empty_request);
    struct QueryResult {
        bool is_empty = false;
        int time = 0;
    };
    std::deque<QueryResult> requests_;
    const static int sec_in_day_ = 1440;
    int no_result_request_count = 0;
    int time = 0;
    const SearchServer&  search_server_;
};