#ifndef DTREE_H
#define DTREE_H

#include <iostream>
#include <map>
#include <string>
#include <vector>

class DTree {
 public:
  typedef enum {Ep, Np, None} PrimType;

  DTree() {};

  void parse(const std::string &line);
  PrimType get_type(int w);
  int get_ep(int w);
  std::string str();

  std::vector<std::vector<int> > eps; // eps[e] = words in e-th ep
  std::vector<int> np; // words in np

 private:
  void parse_eps(const std::string &eps_str);
  void parse_np(const std::string &np_str);
  void parse_words(const std::string &words_str, std::vector<int> &words, DTree::PrimType word_type, int ep_idx = -1);

  std::map<int, int> ep_idx; // ep_idx[w] = index of ep including w
  std::map<int, PrimType> prim_type; // prim_type[w] = primitive type of w
};

#endif
