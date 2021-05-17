#include "test_example_functions.h"

void PrintDocument(const Document& document) {
    std::cout << "{ "
         << "document_id = " << document.id << ", "
         << "relevance = " << document.relevance << ", "
         << "rating = " << document.rating << " }" << std::endl;
}
void PrintMatchDocumentResult(int document_id, const std::vector<std::string>& words, DocumentStatus status) {
    std::cout << "{ "
         << "document_id = " << document_id << ", "
         << "status = " << static_cast<int>(status) << ", "
         << "words =";
    for (const std::string& word : words) {
       std::cout << ' ' << word;
    }
    std::cout << "}" << std::endl;
}

