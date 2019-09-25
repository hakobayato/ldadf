#ifndef LDA_UTILS_H
#define LDA_UTILS_H

#include <string>
#include <vector>

namespace ldautils {
  // print
  void set_verbose(bool flag);
  void comment(std::string s);

  // string
  void split(const std::string &str, char delim, std::vector<std::string> &vec);
  template <typename T> std::string str(T d);

  // math
  template <typename T> T sum(const std::vector<T> &vec);
  template <typename T> T max(const std::vector<T> &vec);
  template <typename T> T min(const std::vector<T> &vec);
  template <typename T> int argmax(const std::vector<T> &vec);
  double digamma(double x);
  
  // prob
  void norm(std::vector<double> &vec);
  int multi(const std::vector<double> &probs);

  // matrix
  template <typename T> void transpose(const std::vector<std::vector<T> > &mat, std::vector<std::vector<T> > &tmat);
  template <typename T> void save_matrix(const std::string &filename, const std::vector<std::vector<T> > &mat);
  template <typename T> void save_matrix_t(const std::string &filename, const std::vector<std::vector<T> > &mat); // with transpose
  void load_matrix(const std::string &filename, std::vector<std::vector<double> > &mat);
};

#endif
