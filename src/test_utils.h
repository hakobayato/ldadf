#include <cxxtest/TestSuite.h>

#include <fstream>
using namespace std;

#include "utils.h"
using namespace ldautils;

class TestUtils : public CxxTest::TestSuite {
  double delta;
  string tmp_file;
  
 public:
  
  void setUp() {
    delta = 0.00001;
    tmp_file = "./test.tmp";
  }

  void tearDown() {
  }

  /* string */

  void test_split() {
    vector<string> list, list2;
    split("1:2  3:4 ", ' ', list);
    //TS_ASSERT_EQUALS((int)list.size(), 3);
    TS_ASSERT_EQUALS((int)list.size(), 4);
    TS_ASSERT_EQUALS(list[0], string("1:2"));
    TS_ASSERT_EQUALS(list[1], string(""));
    TS_ASSERT_EQUALS(list[2], string("3:4"));
    split(list[0], ':', list2); // "1:2"
    TS_ASSERT_EQUALS((int)list2.size(), 2);
    TS_ASSERT_EQUALS(list2[0], string("1"));
    TS_ASSERT_EQUALS(list2[1], string("2"));
    split(list[1], ':', list2); // ""
    //TS_ASSERT_EQUALS((int)list2.size(), 0);
    TS_ASSERT_EQUALS((int)list2.size(), 1);
    split(list[2], ':', list2); // "3:4"
    TS_ASSERT_EQUALS((int)list2.size(), 2);
    TS_ASSERT_EQUALS(list2[0], string("3"));
    TS_ASSERT_EQUALS(list2[1], string("4"));
    split(list[3], ':', list2); // ""
    TS_ASSERT_EQUALS((int)list2.size(), 1);
    
    split("1;", ';', list);
    TS_ASSERT_EQUALS((int)list.size(), 2);
    TS_ASSERT_EQUALS(list[0], "1");
    split(";2", ';', list);
    TS_ASSERT_EQUALS((int)list.size(), 2);
    TS_ASSERT_EQUALS(list[0], "");
    TS_ASSERT_EQUALS(list[1], "2");
  }

  /* math */

  void test_math() {
    double a[] = {2.0, 1.0, 3.0};
    vector<double> vec(a, a+3);
    TS_ASSERT_EQUALS(max(vec), 3.0);
    TS_ASSERT_EQUALS(min(vec), 1.0);
    TS_ASSERT_EQUALS(sum(vec), 6.0);
    TS_ASSERT_EQUALS(argmax(vec), 2);
  }

  void test_digamma() {
    TS_ASSERT_DELTA(digamma(1), -0.57721566, delta);
    TS_ASSERT_DELTA(digamma(5), 1.50611766, delta);
    TS_ASSERT_DELTA(digamma(10), 2.25175258, delta);
  }
  
  /* prob */

  void test_norm() {
    vector<double> vec(2, 1.0);
    norm(vec);
    TS_ASSERT_EQUALS(vec[0], 0.5);
    TS_ASSERT_EQUALS(vec[1], 0.5);
    vector<double> vec2(2, 0.0);
    norm(vec2);
    TS_ASSERT_EQUALS(vec2[0], 0.5);
    TS_ASSERT_EQUALS(vec2[1], 0.5);
  }

  void test_multi() {
    vector<double> vec(2, 0.0);
    vec[0] = 1.0;
    TS_ASSERT_EQUALS(multi(vec), 0);
    vector<double> vec2(10, 0.0);
    vec2[9] = 1.0;
    TS_ASSERT_EQUALS(multi(vec2), 9);
  }

  /* matrix */

  void test_transpose() {
    double matval[2][3] = {
      {1.0, 2.0, 3.0},
      {4.0, 5.0, 6.0}
    };
    vector<vector<double> > mat;
    for(int i = 0; i < 2; ++i) {
      mat.push_back(vector<double>(matval[i], matval[i] + 3));
    }
    vector<vector<double> > tmat;
    transpose(mat, tmat);

    double true_mat[3][2] = {
      {1.0, 4.0},
      {2.0, 5.0},
      {3.0, 6.0}
    };
    TS_ASSERT_EQUALS(static_cast<int>(tmat.size()), 3);
    for(int i = 0; i < 3; ++i) {
      TS_ASSERT_EQUALS(static_cast<int>(tmat[i].size()), 2);
      for(int j = 0; j < 2; ++j) {
	TS_ASSERT_EQUALS(tmat[i][j], true_mat[i][j]);
      }
    }
  }

  void test_save_matrix() {
    double matval[2][3] = {
      {1.1, 1.2, 1.3},
      {2.1, 2.2, 2.3},
    };
    vector<vector<double> > mat;
    for(int i = 0; i < 2; ++i) {
      mat.push_back(vector<double>(matval[i], matval[i] + 3));
    }
    save_matrix(tmp_file, mat);

    string line;
    ifstream file(tmp_file.c_str());
    const char *true_mat[] = {
      "1.1 1.2 1.3 ",
      "2.1 2.2 2.3 "
    };
    for(int i = 0; getline(file, line); ++i) {
      TS_ASSERT_EQUALS(line, true_mat[i]);
    }
  }

  void test_save_matrix_t() {
    double matval[2][3] = {
      {1.1, 1.2, 1.3},
      {2.1, 2.2, 2.3},
    };
    vector<vector<double> > mat;
    for(int i = 0; i < 2; ++i) {
      mat.push_back(vector<double>(matval[i], matval[i] + 3));
    }
    save_matrix_t(tmp_file, mat);

    string line;
    ifstream file(tmp_file.c_str());
    const char *true_mat[] = {
      "1.1 2.1 ",
      "1.2 2.2 ",
      "1.3 2.3 "
    };
    for(int i = 0; getline(file, line); ++i) {
      TS_ASSERT_EQUALS(line, true_mat[i]);
    }
  }

  void test_load_matrix() {
    ofstream file(tmp_file.c_str());
    file << "1.1 1.2 1.3 " << endl;
    file << "2.1 2.2 2.3 " << endl;
    file.close();
    vector<vector<double> > mat;
    load_matrix(tmp_file, mat);

    double true_mat[2][3] = {
      {1.1, 1.2, 1.3},
      {2.1, 2.2, 2.3}
    };
    for(int i = 0; i < mat.size(); ++i) {
      for(int j = 0; j < mat[0].size(); ++j) {
	TS_ASSERT_DELTA(mat[i][j], true_mat[i][j], delta);
      }
    }
  }

};

