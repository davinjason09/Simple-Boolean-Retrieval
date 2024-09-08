#pragma once

#include <string>
#include <unordered_map>
#include <vector>

struct Node {
  char op;
  int size;
  std::string word;
  std::vector<int> ids;

  Node *left;
  Node *right;

  Node(char op) : op(op), left(nullptr), right(nullptr) {}
  Node(const std::string &word) : word(word), left(nullptr), right(nullptr) {}
};

class BooleanRetrieval {
public:
  Node *BuildTree(std::unordered_map<std::string, std::vector<int>> &docIDs,
                  const std::vector<std::string> &queryArray, bool optimize);
  void PrintTree(Node *root, const std::string &prefix = "",
                 bool isLeft = false);
  std::vector<std::string> ParseQuery(const std::string &query);
  std::vector<int> CalculateTree(Node *root, int docSize);

private:
  void ReorderOperands(std::vector<Node *> &operands, std::string &operators,
                       char op);
  void UpdateTree(std::vector<Node *> &operands, std::string &operators,
                  int totalDocuments);
  std::vector<int> CalculateAND(const std::vector<int> &left,
                                const std::vector<int> &right);
  std::vector<int> CalculateOR(const std::vector<int> &left,
                               const std::vector<int> &right);
  std::vector<int> CalculateNOT(const std::vector<int> &right, int docSize);
};
