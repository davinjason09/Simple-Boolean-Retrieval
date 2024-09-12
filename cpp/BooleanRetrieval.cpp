#include "BooleanRetrieval.h"
#include "InvertedIndex.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

Node *BooleanRetrieval::BuildTree(const std::vector<std::string> &tokenArray,
                                  InvertedIndex &index, bool optimize) {
  std::string operators;
  std::vector<Node *> operands;
  int totalDocuments = index.totalDocuments;
  std::unordered_map<std::string, std::vector<int>> docIDs = index.GetDocIDs();

  for (std::string token : tokenArray) {
    if (token == "(") {
      operators.push_back('(');
    } else if (token == ")") {
      while (!operators.empty() && operators.back() != '(') {
        if (optimize) {
          ReorderOperands(operands, operators, '&');
          ReorderOperands(operands, operators, '/');
        }

        UpdateTree(operands, operators, totalDocuments);
      }

      operators.pop_back();
    } else if (token == "NOT" || token == "~") {
      operators.push_back('~');
    } else if (token == "AND" || token == "&") {
      while (!operators.empty() && operators.back() == '~')
        UpdateTree(operands, operators, totalDocuments);

      operators.push_back('&');
    } else if (token == "OR" || token == "/" || token == "|") {
      while (!operators.empty() && operators.back() == '~')
        UpdateTree(operands, operators, totalDocuments);

      while (!operators.empty() && operators.back() == '&') {
        if (optimize)
          ReorderOperands(operands, operators, '&');

        UpdateTree(operands, operators, totalDocuments);
      }

      operators.push_back('/');
    } else {
      std::transform(token.begin(), token.end(), token.begin(), ::tolower);

      Node *node = new Node(token);
      node->ids = docIDs[token];
      node->size = node->ids.size();
      operands.push_back(node);

      while (!operators.empty() && operators.back() == '~')
        UpdateTree(operands, operators, totalDocuments);
    }
  }

  while (!operators.empty()) {
    if (operators.back() == '(')
      std::runtime_error("Parentheses is not balanced");

    if (optimize) {
      ReorderOperands(operands, operators, '&');
      ReorderOperands(operands, operators, '/');
    }

    UpdateTree(operands, operators, totalDocuments);
  };

  if (operands.size() != 1)
    std::runtime_error("Invalid query");

  return operands.back();
}

void BooleanRetrieval::PrintTree(Node *root, const std::string &prefix,
                                 bool isLeft) {
  if (root == nullptr)
    return;

  std::cout << prefix;
  std::cout << (isLeft ? "├──" : "└──");
  std::cout << (root->op ? std::string(1, root->op) : root->word) << '\n';

  std::string newPrefix = prefix + (isLeft ? "│  " : "   ");

  if (root->left || root->right) {
    PrintTree(root->left, newPrefix, true);
    PrintTree(root->right, newPrefix, false);
  }
}

std::vector<std::string>
BooleanRetrieval::ParseQuery(const std::string &query) {
  std::vector<std::string> result;
  std::string token;

  for (char c : query) {
    if (c == '(' || c == ')' || c == ' ' || c == '|' || c == '/' || c == '&' ||
        c == '~') {
      if (!token.empty()) {
        result.push_back(token);
        token.clear();
      }

      if (c != ' ')
        result.push_back(std::string(1, c));
    } else {
      token.push_back(c);
    }
  }

  if (!token.empty())
    result.push_back(token);

  return result;
}

std::vector<int> BooleanRetrieval::CalculateTree(Node *root, int docSize) {
  if (root == nullptr)
    return {};

  if (root->op) {
    std::vector<int> left = CalculateTree(root->left, docSize);
    std::vector<int> right = CalculateTree(root->right, docSize);

    if (root->op == '&')
      return CalculateAND(left, right);
    else if (root->op == '/')
      return CalculateOR(left, right);
    else
      return CalculateNOT(right, docSize);

  } else {
    return root->ids;
  }
}

void BooleanRetrieval::ReorderOperands(std::vector<Node *> &operands,
                                       std::string &operators, char op) {
  int end = operators.size() - 1;
  int length = 1;

  while (operators[end] == op && end >= 0) {
    end--;
    length++;
  }

  if (length <= 2)
    return;

  std::sort(operands.end() - length, operands.end(),
            [](Node *a, Node *b) { return a->size > b->size; });
}

void BooleanRetrieval::UpdateTree(std::vector<Node *> &operands,
                                  std::string &operators, int totalDocuments) {
  char op = operators.back();
  operators.pop_back();

  Node *right = operands.back();
  operands.pop_back();

  Node *node = new Node(op);

  if (op == '~') {
    node->right = right;
    node->size = totalDocuments - right->size;
  } else {
    Node *left = operands.back();
    operands.pop_back();
    node->left = left;
    node->right = right;

    if (op == '&') {
      node->size = std::min(left->size, right->size);
    } else {
      node->size = std::min(totalDocuments, left->size + right->size);
    }
  }

  operands.push_back(node);
}

std::vector<int> BooleanRetrieval::CalculateAND(const std::vector<int> &left,
                                                const std::vector<int> &right) {
  std::vector<int> result;
  result.reserve(std::min(left.size(), right.size()));
  std::set_intersection(left.begin(), left.end(), right.begin(), right.end(),
                        std::back_inserter(result));

  return result;
}

std::vector<int> BooleanRetrieval::CalculateNOT(const std::vector<int> &right,
                                                int docSize) {
  std::vector<int> result;
  std::unordered_set<int> s(right.begin(), right.end());

  for (int i = 0; i < docSize; i++) {
    if (s.find(i) == s.end())
      result.push_back(i);
  }

  return result;
}

std::vector<int> BooleanRetrieval::CalculateOR(const std::vector<int> &left,
                                               const std::vector<int> &right) {
  std::vector<int> result;
  result.reserve(left.size() + right.size());
  set_union(left.begin(), left.end(), right.begin(), right.end(),
            std::back_inserter(result));

  return result;
}
