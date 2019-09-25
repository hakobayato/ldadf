#include "utils.h"

#include <cassert>
#include <cmath>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>
using namespace std;

namespace ldautils {

  /* print */

  static bool verbose_flag = false;

  void
  set_verbose(bool flag) {
    verbose_flag = flag;
  }

  void
  comment(string s) {
    if(ldautils::verbose_flag) {
      cout << s << endl;
    }
  }

  /* string */

  void 
  split(const string &str, char delim, vector<string> &vec) {
    stringstream ss(str);
    string s;
    vec.clear();
    while(getline(ss, s, delim)) {
      vec.push_back(s);
    }
    if(str == "" || *str.rbegin() == delim) { // cf. python's split()
      vec.push_back("");
    }
  }

  template <typename T>
  string
  str(T n) {
    ostringstream ss;
    ss << n;
    return ss.str();
  }

  /* math */

  template <typename T>
  T
  sum(const vector<T> &vec) {
    return accumulate(vec.begin(), vec.end(), static_cast<T>(0));
  }

  template <typename T>
  T
  max(const vector<T> &vec) {
    return *max_element(vec.begin(), vec.end());
  }

  template <typename T>
  T
  min(const vector<T> &vec) {
    return *min_element(vec.begin(), vec.end());
  }

  template <typename T>
  int
  argmax(const vector<T> &vec) {
    return distance(vec.begin(), max_element(vec.begin(), vec.end()));
  }

  // approximation of digamma function
  // cf. http://web.science.mq.edu.au/~mjohnson/code/digamma.c
  double
  digamma(double x) {
    double result = 0, xx, xx2, xx4;
    assert(x > 0);
    for(; x < 7; ++x) {
      result -= 1/x;
    }
    x -= 1.0 / 2.0;
    xx = 1.0 / x;
    xx2 = xx * xx;
    xx4 = xx2 * xx2;
    result += log(x) + (1.0 / 24) * xx2 - (7.0 / 960) * xx4
      + (31.0 / 8064) * xx4 * xx2 - (127.0 / 30720) * xx4 * xx4;
    return result;
  }

  /* prob */

  void
  norm(vector<double> &vec) {
    double s = sum(vec);
    if(s == 0) {
      //cerr << "Warning in ldautils::norm(): sum(vec) is 0" << endl;
      vec.assign(vec.size(), 1.0 / vec.size());
      return;
    }
    double rsum = 1.0 / s;
    for(vector<double>::iterator i = vec.begin(); i != vec.end(); ++i) {
      *i *= rsum;
    }
  }

  const double R_RAND_MAX = 1.0 / RAND_MAX;

  int
  multi(const vector<double> &probs) {
    assert(fabs(sum(probs)-1.0) < 0.0001);
    assert(min(probs) >= 0.0);
    double r = rand() * R_RAND_MAX; // uniform on (0, 1)
    double p = 0;
    int size = probs.size();
    for(int i = 0; i < size; ++i) {
      p += probs[i];
      if(p > r) {
        return i;
      }
    }
    return size-1;
  }

  /* matrix */

  template <typename T>
  void
  transpose(const vector<vector<T> > &mat, vector<vector<T> > &tmat) {
    int row = mat[0].size();
    int col = mat.size();
    tmat.clear();
    for(int i = 0; i < row; ++i) {
      vector<T> trow;
      for(int j = 0; j < col; ++j) {
        trow.push_back(mat[j][i]);
      }
      tmat.push_back(trow);
    }
  }

  template <typename T>
  void
  save_matrix(const string &filename, const vector<vector<T> > &mat) {
    ofstream file(filename.c_str());
    if(!file.is_open()) {
      cerr << "ldautils::save_matrix(): cannot open " << filename << endl;
      exit(1);
    }

    int row = mat.size();
    int col = mat[0].size();
    for(int i = 0; i < row; ++i) {
      assert(mat[i].size() == col);
      for(int j = 0; j < col; ++j) {
        file << mat[i][j] << " ";
      }
      file << endl;
    }
  }

  template <typename T>
  void
  save_matrix_t(const string &filename, const vector<vector<T> > &mat) {
    vector<vector<T> > tmat;
    transpose(mat, tmat);
    save_matrix(filename, tmat);
  }

  void
  load_matrix(const string &filename, vector<vector<double> > &mat) {
    ifstream in(filename.c_str());
    if(!in.is_open()) {
      cerr << "ldautils::load_matrix(): cannot open " << filename << endl;
      exit(1);
    }
  
    mat.clear();
    string line;
    vector<string> strs;
    while(getline(in, line)) {
      split(line, ' ', strs);
      vector<double> vec;
      for(vector<string>::iterator i = strs.begin(); i != strs.end(); ++i) {
        if(*i == "") continue;
        vec.push_back(atof(i->c_str()));
      }
      mat.push_back(vec);
    }
  }

  /* for linking */
  template string str(bool n);
  template string str(int n);
  template string str(double n);
  template int sum(const vector<int>&);
  template double sum(const vector<double>&);
  template int max(const vector<int>&);
  template double max(const vector<double>&);
  template int min(const vector<int>&);
  template double min(const vector<double>&);
  template int argmax(const vector<double>&);
  template void save_matrix(const string&, const vector<vector<int> >&);
  template void save_matrix(const string&, const vector<vector<double> >&);
  template void save_matrix_t(const string&, const vector<vector<int> >&);
  template void save_matrix_t(const string&, const vector<vector<double> >&);
};
