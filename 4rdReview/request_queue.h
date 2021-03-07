#pragma once

#include "search_server.h"
#include <deque>

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server);
    template <typename DocumentPredicate>
    vector<Document> AddFindRequest(const string& raw_query, DocumentPredicate document_predicate) {
        auto documents = search_server_.FindTopDocuments(raw_query, document_predicate);

        AddRequest(documents.empty());

        return documents;
    }

    vector<Document> AddFindRequest(const string& raw_query, DocumentStatus status);
    vector<Document> AddFindRequest(const string& raw_query);
    int GetNoResultRequests() const;
private:
    void AddRequest (bool empty_request);
    void RemoveOldRequests ();
    struct QueryResult {
        bool is_empty = false;
        int time = 0;
    };
    deque<QueryResult> requests_;
    const static int sec_in_day_ = 1440;
    int no_result_request_count = 0;
    int time = 0;
    const SearchServer&  search_server_;
};