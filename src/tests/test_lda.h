#include <cxxtest/TestSuite.h>

#include <fstream>
using namespace std;

#include "../lda.h"
#include "../utils.h"
using namespace ldautils;

class TestLDA : public CxxTest::TestSuite {
  LDA lda;
  double delta;

 public:

  void setUp() {
    string data_file = "../data/test.dat";
    int num_topics = 2;
    double alpha = 0.1;
    double beta = 0.1;
    lda = LDA(data_file, "", num_topics, alpha, beta);
    delta = 0.00001;
  }

  void tearDown() {
  }
  
  void test_initialize() {
    lda.load_data(lda.data_file);
    lda.initialize();
    
    TS_ASSERT_EQUALS(lda.cz.size(), lda.num_topics);
    for(int z = 0; z < lda.num_topics; ++z) {
      TS_ASSERT_EQUALS(lda.cz[z], 0);
    }

    TS_ASSERT_EQUALS(lda.cdz.size(), lda.num_docs);
    for(int d = 0; d < lda.num_docs; ++d) {
      TS_ASSERT_EQUALS(lda.cdz[d].size(), lda.num_topics);
      for(int z = 0; z < lda.num_topics; ++z) {
        TS_ASSERT_EQUALS(lda.cdz[d][z], 0);
      }
    }

    TS_ASSERT_EQUALS(lda.cwz.size(), lda.num_words);
    for(int w = 0; w < lda.num_words; ++w) {
      TS_ASSERT_EQUALS(lda.cwz[w].size(), lda.num_topics);
      for(int z = 0; z < lda.num_topics; ++z) {
        TS_ASSERT_EQUALS(lda.cwz[w][z], 0);
      }
    }

    TS_ASSERT_EQUALS(lda.hz.size(), lda.num_docs);
    for(int d = 0; d < lda.num_docs; ++d) {
      TS_ASSERT_EQUALS(lda.hz[d].size(), lda.nd[d]);
      for(int i = 0; i < lda.nd[d]; ++i) {
        TS_ASSERT_EQUALS(lda.hz[d][i], 0);
      }
    }

    TS_ASSERT_EQUALS(lda.alphas.size(), lda.num_topics);
    for(int z = 0; z < lda.num_topics; ++z) {
      TS_ASSERT_DELTA(lda.alphas[z], lda.alpha, delta);
    }

    TS_ASSERT_EQUALS(lda.betas.size(), lda.num_words);
    for(int w = 0; w < lda.num_words; ++w) {
      TS_ASSERT_DELTA(lda.betas[w], lda.beta, delta);
    }
    
    TS_ASSERT_EQUALS(lda.probs.size(), lda.num_topics);
    TS_ASSERT_EQUALS(lda.phi.size(), lda.num_topics);
    for(int z = 0; z < lda.num_topics; ++z) {
      TS_ASSERT_EQUALS(lda.phi[z].size(), lda.num_words);
    }
    TS_ASSERT_EQUALS(lda.theta.size(), lda.num_docs);
    for(int d = 0; d < lda.num_docs; ++d) {
      TS_ASSERT_EQUALS(lda.theta[d].size(), lda.num_topics);
    }
  }

  void test_preprocess() {
    lda.load_data(lda.data_file);
    lda.initialize();
    lda.preprocess();

    TS_ASSERT_EQUALS(sum(lda.cz), lda.num_terms);

    int sum_cdz = 0;
    for(int d = 0; d < lda.num_docs; ++d) {
      TS_ASSERT_EQUALS(sum(lda.cdz[d]), lda.nd.size());
      sum_cdz += sum(lda.cdz[d]);
    }
    TS_ASSERT_EQUALS(sum_cdz, lda.num_terms);

    int sum_cwz = 0;
    for(int w = 0; w < lda.num_words; ++w) {
      sum_cwz += sum(lda.cwz[w]);
    }
    TS_ASSERT_EQUALS(sum_cwz, lda.num_terms);
  }

  void test_load_data() {
    lda.load_data(lda.data_file);
    TS_ASSERT_EQUALS(lda.docs.size(), 4);
    TS_ASSERT_EQUALS(lda.docs[0].size(), 4);
    TS_ASSERT_EQUALS(lda.docs[1].size(), 4);
    TS_ASSERT_EQUALS(lda.docs[0][0], 0);
    TS_ASSERT_EQUALS(lda.docs[0][1], 0);
    TS_ASSERT_EQUALS(lda.docs[0][2], 1);
    TS_ASSERT_EQUALS(lda.docs[0][3], 1);
    TS_ASSERT_EQUALS(lda.docs[1][0], 0);
    TS_ASSERT_EQUALS(lda.docs[1][1], 0);
    TS_ASSERT_EQUALS(lda.docs[1][2], 2);
    TS_ASSERT_EQUALS(lda.docs[1][3], 2);
    
    TS_ASSERT_EQUALS(lda.num_docs, 4);
    TS_ASSERT_EQUALS(lda.num_words, 3);
    TS_ASSERT_EQUALS(lda.num_terms, 16);

    TS_ASSERT_EQUALS(lda.nd.size(), 4);
    TS_ASSERT_EQUALS(lda.nd[0], 4);
    TS_ASSERT_EQUALS(lda.nd[1], 4);
    TS_ASSERT_EQUALS(lda.nd[2], 4);
    TS_ASSERT_EQUALS(lda.nd[3], 4);
  }

  void test_resample_pre() {
    lda.load_data(lda.data_file);
    lda.initialize();
    lda.preprocess();

    int cz = lda.cz[0];
    int cdz = lda.cdz[0][0];
    int cwz = lda.cwz[0][0];
    lda.resample_pre(0, 0, 0);

    TS_ASSERT_EQUALS(lda.cz[0], cz - 1);
    TS_ASSERT_EQUALS(lda.cdz[0][0], cdz - 1);
    TS_ASSERT_EQUALS(lda.cwz[0][0], cwz - 1);
  }

  void test_resample_post() {
    lda.load_data(lda.data_file);
    lda.initialize();
    lda.preprocess();

    int cz = lda.cz[0];
    int cdz = lda.cdz[0][0];
    int cwz = lda.cwz[0][0];
    lda.resample_post(0, 0, 0);

    TS_ASSERT_EQUALS(lda.cz[0], cz + 1);
    TS_ASSERT_EQUALS(lda.cdz[0][0], cdz + 1);
    TS_ASSERT_EQUALS(lda.cwz[0][0], cwz + 1);
  }

  void test_calc_probs() {
    lda.load_data(lda.data_file);
    lda.initialize();

    // toy sample
    lda.cz[0] = lda.num_terms;
    lda.cdz[0][0] = lda.num_terms;
    lda.cwz[0][0] = lda.num_terms;

    lda.calc_probs(0, 0, lda.probs);

    double alpha = lda.alpha;
    double beta = lda.alpha;
    int num_terms = lda.num_terms;
    int num_words = lda.num_words;
    int num_topics = lda.num_topics;

    vector<double> true_probs(num_topics, (0 + beta) * (0 + alpha) / (0 + beta * num_words));
    true_probs[0] = (num_terms + beta) * (num_terms + alpha) / (num_terms + beta * num_words);
    norm(true_probs);

    for(int z = 0; z < lda.num_topics; ++z) {
      TS_ASSERT_DELTA(lda.probs[z], true_probs[z], delta);
    }
  }

  void test_calc_perplexity() {
    lda.load_data(lda.data_file);
    lda.initialize();

    // toy sample
    lda.cz[0] = lda.num_terms;
    lda.cdz[0][0] = lda.num_terms;
    lda.cwz[0][0] = lda.num_terms;

    double pp = lda.calc_perplexity();
    TS_ASSERT_DELTA(pp, 4.15282, delta);
  }

  void test_get_phi_theta() {
    lda.load_data(lda.data_file);
    lda.initialize();

    // toy sample
    lda.cz[0] = lda.num_terms;
    lda.cdz[0][0] = lda.num_terms;
    lda.cwz[0][0] = lda.num_terms;

    double alpha = lda.alpha;
    double beta = lda.alpha;
    double num_terms = lda.num_terms;
    double num_words = lda.num_words;
    double num_topics = lda.num_topics;

    double true_phi[2][3] = {
      {(num_terms + beta) / (num_terms + num_words * beta),
       (0 + beta)/(num_terms + num_words * beta),
       (0 + beta)/(num_terms + num_words * beta)
      },
      {(0 + beta)/(0 + num_words * beta),
       (0 + beta)/(0 + num_words * beta),
       (0 + beta)/(0 + num_words * beta)
      },
    };

    lda.get_phi(lda.phi);
    for(int z = 0; z < num_topics; ++z) {
      TS_ASSERT_DELTA(sum(lda.phi[z]), 1.0, delta);
      for(int w = 0; w < lda.num_words; ++w) {
	TS_ASSERT_DELTA(lda.phi[z][w], true_phi[z][w], delta);
      }
    }

    double true_theta[4][2] = {
      {(num_terms + alpha) / (num_terms + num_topics * alpha),
       (0 + alpha) / (num_terms + num_topics * alpha)
      },
      {(0 + alpha) / (0 + num_topics * alpha),
       (0 + alpha) / (0 + num_topics * alpha)
      },
      {(0 + alpha) / (0 + num_topics * alpha),
       (0 + alpha) / (0 + num_topics * alpha)
      },
      {(0 + alpha) / (0 + num_topics * alpha),
       (0 + alpha) / (0 + num_topics * alpha)
      },
    };

    lda.get_theta(lda.theta);
    for(int d = 0; d < lda.num_docs; ++d) {
      TS_ASSERT_DELTA(sum(lda.theta[d]), 1.0, delta);
      for(int z = 0; z < num_topics; ++z) {
	TS_ASSERT_DELTA(lda.theta[d][z], true_theta[d][z], delta);
      }
    }
  }
};
