#include <cxxtest/TestSuite.h>

#include "dtree.h"

class TestDTree : public CxxTest::TestSuite {
  DTree dt;

public:

  void setUp() {
  }
  
  void tearDown() {
  }

  void test_parse() {
    dt.parse("0,1:2,3;4,5");
    TS_ASSERT_EQUALS(dt.eps.size(), 2);
    TS_ASSERT_EQUALS(dt.eps[0].size(), 2);
    TS_ASSERT_EQUALS(dt.eps[0][0], 0);
    TS_ASSERT_EQUALS(dt.eps[0][1], 1);
    TS_ASSERT_EQUALS(dt.eps[1].size(), 2);
    TS_ASSERT_EQUALS(dt.eps[1][0], 2);
    TS_ASSERT_EQUALS(dt.eps[1][1], 3);
    TS_ASSERT_EQUALS(dt.np.size(), 2);
    TS_ASSERT_EQUALS(dt.np[0], 4);
    TS_ASSERT_EQUALS(dt.np[1], 5);

    dt.parse(";");
    TS_ASSERT_EQUALS(dt.eps.size(), 0);
    TS_ASSERT_EQUALS(dt.np.size(), 0);

    dt.parse("0,1,2;");
    TS_ASSERT_EQUALS(dt.eps.size(), 1);
    TS_ASSERT_EQUALS(dt.eps[0].size(), 3);
    TS_ASSERT_EQUALS(dt.eps[0][0], 0);
    TS_ASSERT_EQUALS(dt.eps[0][1], 1);
    TS_ASSERT_EQUALS(dt.eps[0][2], 2);
    TS_ASSERT_EQUALS(dt.np.size(), 0);

    dt.parse(";0,1,2");
    TS_ASSERT_EQUALS(dt.eps.size(), 0);
    TS_ASSERT_EQUALS(dt.np.size(), 3);
    TS_ASSERT_EQUALS(dt.np[0], 0);
    TS_ASSERT_EQUALS(dt.np[1], 1);
    TS_ASSERT_EQUALS(dt.np[2], 2);
  }

  void test_str() {
    dt.parse("0,1:2,3;4,5");
    TS_ASSERT_EQUALS(dt.str(), "Ep(0,1)^Ep(2,3)^Np(4,5)");

    dt.parse(";");
    TS_ASSERT_EQUALS(dt.str(), "");

    dt.parse("0,1,2;");
    TS_ASSERT_EQUALS(dt.str(), "Ep(0,1,2)");

    dt.parse(";0,1,2");
    TS_ASSERT_EQUALS(dt.str(), "Np(0,1,2)");
  }
};
