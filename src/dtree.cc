#include "dtree.h"

#include <cassert>
#include <cmath>
#include <cstdlib>

#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

#include "./utils.h"
using namespace ldautils;

void
DTree::parse(const string &line) {
  ep_idx.clear();
  prim_type.clear();
  
  vector<string> conj;
  split(line, ';', conj);
  assert(conj.size() == 2);

  string eps_str = conj[0];
  string np_str = conj[1];
  parse_eps(eps_str);
  parse_np(np_str);
}

DTree::PrimType
DTree::get_type(int w) {
  if(prim_type.count(w) > 0) {
    return prim_type[w]; // Ep or Np
  }
  return None;
}

int
DTree::get_ep(int w) {
  assert(ep_idx.count(w) > 0);
  return ep_idx[w];
}

string
DTree::str() {
  ostringstream ss;
  bool conj_flag = false;
  for(vector<vector<int> >::const_iterator i = eps.begin(); i != eps.end(); ++i) {
    if(conj_flag) ss << "^";
    conj_flag = true;
    ss << "Ep(";
    for(vector<int>::const_iterator j = i->begin(); j != i->end(); ++j) {
      if(j != i->begin()) ss << ",";
      ss << *j;
    }
    ss << ")";
  }
  if(np.size() > 0) {
    if(conj_flag) ss << "^";
    ss << "Np(";
    for(vector<int>::const_iterator i = np.begin(); i != np.end(); ++i) {
      if(i != np.begin()) ss << ",";
      ss << *i;
    }
    ss << ")";
  }
  return ss.str();
}

void
DTree::parse_eps(const string &eps_str) {
  eps.clear();
  vector<string> ep_strs;
  split(eps_str, ':', ep_strs); 
  for(int i = 0; i < ep_strs.size(); ++i) {
    if(ep_strs[i] == "") continue;
    vector<int> ep;
    parse_words(ep_strs[i], ep, Ep, i);
    eps.push_back(ep);
  }
}

void
DTree::parse_np(const string &np_str) {
  parse_words(np_str, np, Np);
}

void
DTree::parse_words(const string &words_str, vector<int> &words, DTree::PrimType type, int idx) {
  words.clear();
  vector<string> ws;
  split(words_str, ',', ws);
  for(vector<string>::iterator i = ws.begin(); i != ws.end(); ++i) {
    if(*i == "") continue;
    int w = atoi(i->c_str());
    if (w == 0 && *i != "0") {
      cerr << "DTree::parse_words(): unknown character " << *i << endl;
      exit(1);
    }
    words.push_back(w);
    prim_type[w] = type;
    if(idx >= 0) { // ignore Np
      ep_idx[w] = idx;
    }
  }  
}
