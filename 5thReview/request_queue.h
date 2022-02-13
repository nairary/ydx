#pragma once

#include "search_server.h"

#include <deque>


//Хранит в себе класс RequestQueue, принимающий запросы на поиск "AddFindRequest",
//сам класс RequestQueue отвечает сколько запросов за последние сутки (1440 минут) отслось без результата

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server)
    : search_server_(search_server)
    , no_results_requests_(0)
    , current_time_(0)
    {}

    //Объявление методов AddFindRequest
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
    std::vector<Document> AddFindRequest(const std::string& raw_query);

    //Реализация шаблонного метода AddFindRequest
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
        const auto result = search_server_.FindTopDocuments(raw_query, document_predicate);
        AddRequest(result.size());
        return result;
    }

    int GetNoResultRequests() const;

private:
    struct QueryResult {
        uint64_t timestamp;
        int results;
    };
    std::deque<QueryResult> requests_;
    const SearchServer& search_server_;
    int no_results_requests_;
    uint64_t current_time_;
    constexpr static int sec_in_day_ = 1440;

    void AddRequest(int results_num);
};

