#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class InvertedIndex {
public:
  size_t index;
  std::string current;
  int totalDocuments;

  void ReadCSV(const std::string &filePath, const std::string &delimiter);
  void ReadID(const std::string &content);
  void ReadFromBack(const std::string &content);

  std::unordered_map<std::string, std::vector<int>> GetDocIDs() {
    return docIDs;
  };

private:
  bool IsMailNumber(const std::string &content, size_t index);
  bool IsCurrency(const std::string &content, const std::string &word,
                  size_t index);
  void ParseContent(const std::string &content, int is);
  void AddToDocIDs(std::string &word, int docID);
  std::unordered_map<std::string, std::vector<int>> docIDs;
};
