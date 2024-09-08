#include "BooleanRetrieval.h"
#include "InvertedIndex.h"
#include <atomic>
#include <fstream>
#include <iostream>
#include <thread>

std::atomic<bool> indexReady(false);

void buildIndex(InvertedIndex &index, const std::string &filePath,
                const std::string &delimiter) {
  index.ReadCSV(filePath, delimiter);
  indexReady = true;
}

int main(int argc, char *argv[]) {
  InvertedIndex index;

  std::ofstream fout("output.txt");
  std::string filePath = "News.csv";
  std::string delimiter = ",";

  std::thread indexThread(buildIndex, std::ref(index), filePath, delimiter);

  BooleanRetrieval model;
  std::string query;

  std::cout << "Enter query: ";
  getline(std::cin, query);

  std::vector<std::string> queryArray = model.ParseQuery(query);

  if (!indexReady)
    std::cout << "Building index...\n";

  indexThread.join();

  std::unordered_map<std::string, std::vector<int>> docIDs = index.GetDocIDs();
  fout << "Dict size:" << docIDs.size() << '\n';

  for (auto &it : docIDs)
    fout << "'" << it.first << "' : " << it.second.size() << '\n';

  Node *root = model.BuildTree(docIDs, queryArray, false);

  std::cout << '\n';
  model.PrintTree(root);
  std::vector<int> result = model.CalculateTree(root, docIDs.size());

  std::cout << "\ndocID: {  " << (result.size() ? '\b' : ' ');
  for (int id : result)
    std::cout << id << ", ";

  std::cout << "\b\b }\n";
}
