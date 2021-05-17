#pragma once
#include "document.h"

#include <vector>
#include <string>

void PrintDocument(const Document& document);
void PrintMatchDocumentResult(int document_id, const std::vector<std::string>& words, DocumentStatus status);
