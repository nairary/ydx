#include "request_queue.h"

RequestQueue::RequestQueue(const SearchServer& search_server)
    : search_server_(search_server){
}
    vector<Document> RequestQueue::AddFindRequest(const string& raw_query, DocumentStatus status) {
        auto documents = search_server_.FindTopDocuments(raw_query, status);

        AddRequest(documents.empty());

        return documents;
    }

    vector<Document> RequestQueue::AddFindRequest(const string& raw_query) {
        auto documents = search_server_.FindTopDocuments(raw_query);

        AddRequest(documents.empty());

        return documents;
    }

    int RequestQueue::GetNoResultRequests() const {
        return no_result_request_count;
    }

    void RequestQueue::AddRequest (bool empty_request) {
        ++time;
        QueryResult query_result;
        query_result.is_empty = empty_request;
        query_result.time = time;

        requests_.push_back(query_result);
        if (query_result.is_empty) {
            ++no_result_request_count;
        }
        RequestQueue::RemoveOldRequests();
    }
    void RequestQueue::RemoveOldRequests (){
        if (requests_.empty()){
            return;
        }
        int out_dated_requests = requests_.back().time - requests_.front().time + 1 - sec_in_day_;
        if (out_dated_requests <= 0) {
            return;
        }
        for (auto i = out_dated_requests;i > 0; --i) {
            if (requests_.front().is_empty) {
                --no_result_request_count;
            }
            requests_.pop_front();
        }
    }


