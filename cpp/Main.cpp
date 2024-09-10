#include "BooleanRetrieval.h"
#include "InvertedIndex.h"
#include <atomic>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

std::atomic<bool> indexReady(false);
double indexTime;
double queryTime;
double optimizedQueryTime;

void buildIndex(InvertedIndex &index, const std::string &filePath,
                const std::string &delimiter) {
  auto start = std::chrono::high_resolution_clock::now();
  index.ReadCSV(filePath, delimiter);
  auto end = std::chrono::high_resolution_clock::now();

  indexTime = std::chrono::duration<double>(end - start).count();
  indexReady = true;
}

int main(int argc, char *argv[]) {
  InvertedIndex index;

  std::ofstream fout("output.txt");
  std::string filePath = "../data/News.csv";
  std::string delimiter = ",";

  std::thread indexThread(buildIndex, std::ref(index), filePath, delimiter);

  BooleanRetrieval model;
  std::string query;

  std::cout << "Enter query: ";
  getline(std::cin, query);

  std::vector<std::string> tokenArray = model.ParseQuery(query);

  if (!indexReady)
    std::cout << "Building index...\n";

  indexThread.join();

  std::unordered_map<std::string, std::vector<int>> docIDs = index.GetDocIDs();

  for (auto &it : docIDs) {
    fout << it.first << ": { ";
    for (int id : it.second)
      fout << id << ", ";
    fout << "}\n";
  }

  auto start = std::chrono::high_resolution_clock::now();
  Node *root = model.BuildTree(tokenArray, index, false);
  std::vector<int> result = model.CalculateTree(root, index.totalDocuments);
  auto end = std::chrono::high_resolution_clock::now();
  queryTime = std::chrono::duration<double>(end - start).count();

  start = std::chrono::high_resolution_clock::now();
  Node *rootOptimized = model.BuildTree(tokenArray, index, true);
  std::vector<int> resultOptimized =
      model.CalculateTree(root, index.totalDocuments);
  end = std::chrono::high_resolution_clock::now();
  optimizedQueryTime = std::chrono::duration<double>(end - start).count();

  std::cout << '\n';
  std::cout << "Tree:\n";
  model.PrintTree(root);

  std::cout << "\nOptimized tree:\n";
  model.PrintTree(rootOptimized);

  std::cout << "\nAmount: " << result.size() << '\n';
  /*std::cout << "\ndocID: {  ";*/
  /*for (int id : result)*/
  /*  std::cout << id << ", ";*/
  /**/
  /*std::cout << "}\n";*/

  std::cout << "\n\nStatistics:\n";
  std::cout << "Indexing time: " << indexTime << "s\n";
  std::cout << "Total Documents: " << index.totalDocuments << '\n';
  std::cout << "Total Terms: " << docIDs.size() << '\n';
  std::cout << "Query time: " << queryTime << "s\n";
  std::cout << "Optimized query time: " << optimizedQueryTime << "s\n";
}
