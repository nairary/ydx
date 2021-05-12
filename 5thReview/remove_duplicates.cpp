#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "search_server.h"

void RemoveDuplicates(SearchServer& search_server){
    std::map<std::map<std::string, double>, int> docs;
    std::set<int> ids_to_remove;
    for (int document_id : search_server) {
        std::map<std::string, double> doc_frequencies =
                search_server.GetWordFrequencies(document_id);
        for (auto& [word, freq] : doc_frequencies) {
            freq = 0;
        }
        if (!docs.emplace(doc_frequencies, document_id).second) {
            ids_to_remove.insert(document_id);
        }
    }
    for (auto id : ids_to_remove) {
        std::cout << "Found duplicate document id " << id << std::endl;
        search_server.RemoveDocument(id);
    }
}