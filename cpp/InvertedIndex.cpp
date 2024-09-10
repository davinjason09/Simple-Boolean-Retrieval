#include "InvertedIndex.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>

void InvertedIndex::ReadID(const std::string &content) {
  current.clear();

  while (index < content.size() && content[index] != ',') {
    current.push_back(content[index]);
    index++;
  }
}

void InvertedIndex::ReadFromBack(const std::string &content) {
  current.clear();
  bool inQuotes = false;

  while (index > 0) {
    char c = content[index];

    if (c == ',' && !inQuotes) {
      index--;
      return;
    }

    if (c == '"')
      inQuotes = !inQuotes;

    current.push_back(c);
    index--;
  }
}

bool InvertedIndex::IsMailNumber(const std::string &content, size_t index) {
  return (index > 0 && index < content.size() - 1 &&
          (isdigit(content[index - 1]) || isupper(content[index - 1])) &&
          (isdigit(content[index + 1]) || isupper(content[index + 1])));
}

bool InvertedIndex::IsCurrency(const std::string &content,
                               const std::string &word, size_t index) {
  bool isCurrency = (word.back() == '.' && isdigit(content[index - 2]) &&
                     content[index - 3] != ' ');

  if (isCurrency)
    return isdigit(content[index + 1]) && isdigit(content[index + 2]) &&
           isdigit(content[index + 3]);

  return false;
}

void InvertedIndex::AddToDocIDs(std::string &word, int docID) {
  if (word.size() && (docIDs[word].empty() || docIDs[word].back() != docID))
    docIDs[word].push_back(docID);

  word.clear();
}

void InvertedIndex::ParseContent(const std::string &content, int id) {
  std::string cleanWord;
  size_t size = content.size();
  bool mailNumber = false;

  for (size_t i = 0; i < size; i++) {
    char c = content[i];

    if (c == ' ') {
      if (i + 1 < size &&
          IsCurrency(content, cleanWord, (content[i - 1] == ' ' ? i - 1 : i))) {
        continue;
      } else {
        if (cleanWord.back() == '.' || cleanWord.back() == '-')
          cleanWord.pop_back();

        AddToDocIDs(cleanWord, id);
        mailNumber = false;
      }
    } else if (c == '/' && IsMailNumber(content, i)) {
      mailNumber = true;
      cleanWord.push_back(c);
    } else if (i + 1 > size && (c == '-' && content[i + 1] != '-') ||
               (c == '.' && (isdigit(content[i + 1]) || mailNumber))) {
      cleanWord.push_back(c);
    } else if (isalnum(c)) {
      if (cleanWord.back() == '.' && isalpha(c) && !mailNumber) {
        AddToDocIDs(cleanWord, id);
        mailNumber = false;
      }

      cleanWord.push_back(tolower(c));
    } else {
      if (cleanWord.back() == '.' || cleanWord.back() == '-')
        cleanWord.pop_back();

      AddToDocIDs(cleanWord, id);
      mailNumber = false;
    }
  }

  if (!cleanWord.empty()) {
    if (cleanWord.back() == '.' || cleanWord.back() == '-')
      cleanWord.pop_back();

    AddToDocIDs(cleanWord, id);
  }
}

void InvertedIndex::ReadCSV(const std::string &filePath,
                            const std::string &delimiter) {
  std::ifstream file(filePath);

  if (!file.is_open()) {
    std::cerr << "Error: file not found\n";
    return;
  }

  std::string line;
  int id;
  getline(file, line);

  int total = 1;
  while (getline(file, line)) {
    index = 0;
    ReadID(line);
    int id = std::stoi(current);

    index = line.size() - 1;
    ReadFromBack(line);
    ReadFromBack(line);
    std::string content = current;

    std::reverse(content.begin(), content.end());
    ParseContent(content, id);
    total++;
  }

  totalDocuments = total;
}
