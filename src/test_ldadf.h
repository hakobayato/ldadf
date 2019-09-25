#include <cxxtest/TestSuite.h>

#include "ldadf.h"
#include "utils.h"

using namespace std;

class TestLDADF : public CxxTest::TestSuite {
  LDADF lda;
  double delta;
  string tmp_file;

public:
  
  void setUp() {
    string dat_file = "../data/test.dat";
    string out_base = "";
    string dnf_file = "../data/test.dnf";
    int num_topics = 2;
    int max_steps = 10;
    int num_loops = 10;
    int burn_in = 5;
    int seed = 0;
    double alpha = 1.0;
    double beta = 0.01;
    double eta = 10;
    bool converge = false;
    bool verbose = false;
    lda = LDADF(dat_file, out_base, num_topics, alpha, beta,
                max_steps, num_loops, burn_in, converge, seed, verbose,
                dnf_file, eta);
    delta = 0.00001;
    tmp_file = "./test.tmp";
  }
  
  void tearDown() {
  }

  void test_initialize() {
    lda.initialize();

    TS_ASSERT_EQUALS(lda.ctz.size(), lda.num_dtrees);
    TS_ASSERT_EQUALS(lda.ctze.size(), lda.num_dtrees);
    for(int t = 0; t < lda.num_dtrees; ++t) {
      TS_ASSERT_EQUALS(lda.ctz[t].size(), lda.num_topics);
      TS_ASSERT_EQUALS(lda.ctze[t].size(), lda.num_topics);
      for(int z = 0; z < lda.num_topics; ++z) {
        TS_ASSERT_EQUALS(lda.ctz[t][z], 0);
        TS_ASSERT_EQUALS(lda.ctze[t][z].size(), lda.dtrees[t].eps.size());
        for(int e = 0; e < lda.dtrees[t].eps.size(); ++e) {
          TS_ASSERT_EQUALS(lda.ctze[t][z][e], 0);
        }
      }
    }

    TS_ASSERT_EQUALS(lda.dz.size(), lda.num_topics);
  }

  void test_preprocess() {
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

    for(int t = 0; t < lda.num_dtrees; ++t) {
      DTree &dt = lda.dtrees[t];
      for(int z = 0; z < lda.num_topics; ++z) {
        int ctz = 0;
        vector<int> ctze(lda.ctze[t][z].size(), 0);
        for(int w = 0; w < lda.num_words; ++w) {
          int e;
          switch(dt.get_type(w)) {
          case DTree::Ep:
            e = dt.get_ep(w);
            ctze[e] += lda.cwz[w][z];
            ctz += lda.cwz[w][z];
            break;
          case DTree::None:
            ctz += lda.cwz[w][z];
            break;
          }
        }
        TS_ASSERT_EQUALS(ctz, lda.ctz[t][z]);
        for(int e = 0; e < dt.eps.size(); ++e) {
          TS_ASSERT_EQUALS(ctze[e], lda.ctze[t][z][e]);
        }
      }
    }
  }

  void test_resample() {
    lda.initialize();
    lda.preprocess();
    lda.resample();
    for(int z = 0; z < lda.num_topics; ++z) {
      TS_ASSERT(lda.dz[z] >= 0);
      TS_ASSERT(lda.dz[z] < lda.num_dtrees);
    }
  }

  void test_resample_post_pre() {
    lda.initialize();
    lda.dz[0] = 0;
    lda.dz[1] = 1;

    // fixed sampling
    for(int i = 0; i < 4; ++i) {
      lda.resample_post(0, 0, 1);
      lda.resample_post(1, 0, 0);
      lda.resample_post(2, 1, 1);
      lda.resample_post(3, 2, 0);
    }

    TS_ASSERT_EQUALS(lda.cdz[0][1], 4);
    TS_ASSERT_EQUALS(lda.cdz[1][0], 4);
    TS_ASSERT_EQUALS(lda.cdz[2][1], 4);
    TS_ASSERT_EQUALS(lda.cdz[3][0], 4);
    TS_ASSERT_EQUALS(lda.cdz[0][0], 0);
    TS_ASSERT_EQUALS(lda.cdz[1][1], 0);
    TS_ASSERT_EQUALS(lda.cdz[2][0], 0);
    TS_ASSERT_EQUALS(lda.cdz[3][1], 0);
    TS_ASSERT_EQUALS(lda.cz[0], 8);
    TS_ASSERT_EQUALS(lda.cz[1], 8);
    TS_ASSERT_EQUALS(lda.cwz[0][0], 4);
    TS_ASSERT_EQUALS(lda.cwz[0][1], 4);
    TS_ASSERT_EQUALS(lda.cwz[1][0], 0);
    TS_ASSERT_EQUALS(lda.cwz[1][1], 4);
    TS_ASSERT_EQUALS(lda.cwz[2][0], 4);
    TS_ASSERT_EQUALS(lda.cwz[2][1], 0);

    TS_ASSERT_EQUALS(lda.ctz[0][0], 4);
    TS_ASSERT_EQUALS(lda.ctze[0][0].size(), 0);
    TS_ASSERT_EQUALS(lda.ctz[1][1], 8);
    TS_ASSERT_EQUALS(lda.ctze[1][1].size(), 1);
    TS_ASSERT_EQUALS(lda.ctze[1][1][0], 8);

    // reversed fixed-sampling
    for(int i = 0; i < 4; ++i) {
      lda.resample_pre(0, 0, 1);
      lda.resample_pre(1, 0, 0);
      lda.resample_pre(2, 1, 1);
      lda.resample_pre(3, 2, 0);
    }

    TS_ASSERT_EQUALS(lda.cdz[0][1], 0);
    TS_ASSERT_EQUALS(lda.cdz[1][0], 0);
    TS_ASSERT_EQUALS(lda.cdz[2][1], 0);
    TS_ASSERT_EQUALS(lda.cdz[3][0], 0);
    TS_ASSERT_EQUALS(lda.cdz[0][0], 0);
    TS_ASSERT_EQUALS(lda.cdz[1][1], 0);
    TS_ASSERT_EQUALS(lda.cdz[2][0], 0);
    TS_ASSERT_EQUALS(lda.cdz[3][1], 0);
    TS_ASSERT_EQUALS(lda.cz[0], 0);
    TS_ASSERT_EQUALS(lda.cz[1], 0);
    TS_ASSERT_EQUALS(lda.cwz[0][0], 0);
    TS_ASSERT_EQUALS(lda.cwz[0][1], 0);
    TS_ASSERT_EQUALS(lda.cwz[1][0], 0);
    TS_ASSERT_EQUALS(lda.cwz[1][1], 0);
    TS_ASSERT_EQUALS(lda.cwz[2][0], 0);
    TS_ASSERT_EQUALS(lda.cwz[2][1], 0);

    TS_ASSERT_EQUALS(lda.ctz[0][0], 0);
    TS_ASSERT_EQUALS(lda.ctze[0][0].size(), 0);
    TS_ASSERT_EQUALS(lda.ctz[1][1], 0);
    TS_ASSERT_EQUALS(lda.ctze[1][1].size(), 1);
    TS_ASSERT_EQUALS(lda.ctze[1][1][0], 0);
  }

  void test_load_dnf() {
    lda.load_dnf(lda.dnf_file);
    TS_ASSERT_EQUALS(lda.dtrees.size(), 2);
    TS_ASSERT_EQUALS(lda.dtrees[0].eps.size(), 0);
    TS_ASSERT_EQUALS(lda.dtrees[0].np.size(), 2);
    TS_ASSERT_EQUALS(lda.dtrees[0].np[0], 0);
    TS_ASSERT_EQUALS(lda.dtrees[0].np[1], 1);
    TS_ASSERT_EQUALS(lda.dtrees[1].eps.size(), 1);
    TS_ASSERT_EQUALS(lda.dtrees[1].eps[0].size(), 2);
    TS_ASSERT_EQUALS(lda.dtrees[1].eps[0][0], 0);
    TS_ASSERT_EQUALS(lda.dtrees[1].eps[0][1], 1);
    TS_ASSERT_EQUALS(lda.dtrees[1].np.size(), 1);
    TS_ASSERT_EQUALS(lda.dtrees[1].np[0], 2);
  }

  void test_calc_dtree_probs() {
    lda.initialize();
    lda.dz[0] = 0;
    lda.dz[1] = 1;

    // fixed sampling
    for(int i = 0; i < 4; ++i) {
      lda.resample_post(0, 0, 1);
      lda.resample_post(0, 0, 0);
      lda.resample_post(0, 1, 1);
      lda.resample_post(0, 2, 0);
    }

    double beta = lda.beta;
    double eta = lda.eta;
    int num_np, num_nonp, num_ep;

    vector<double> true_probs(2);
    vector<double> probs(2);

    // dtree probs when z = 0
    lda.calc_dtree_probs(0, probs);

    // true prob when z = 0 and dnf = ";0,1"
    num_np = 2;
    num_nonp = 1;
    num_ep = 0;
    true_probs[0] = num_nonp
      // from root node
      * tgamma(beta * eta * num_nonp + beta * num_np) / tgamma(8 + beta * eta * num_nonp + beta * num_np)
      * tgamma(4 + beta * eta * num_nonp) / tgamma(beta * eta * num_nonp) // non-np node
      * tgamma(4 + beta) / tgamma(beta) // Np(0)
      * tgamma(0 + beta) / tgamma(beta) // Np(1)
      // from non-np node
      * tgamma(beta * num_nonp) / tgamma(4 + beta * num_nonp)
      * tgamma(4 + beta) / tgamma(beta); // normal leaf 2

    // true prob when z = 0 and dnf = "0,1;2"
    num_np = 1;
    num_nonp = 2;
    num_ep = 2;
    true_probs[1] = num_nonp
      // from root node
      * tgamma(beta * eta * num_nonp + beta * num_np) / tgamma(8 + beta * eta * num_nonp + beta * num_np)
      * tgamma(4 + beta * eta * num_nonp) / tgamma(beta * eta * num_nonp) // non-np node
      * tgamma(4 + beta) / tgamma(beta) // Np(2)
      // from non-np node
      * tgamma(beta * num_nonp) / tgamma(4 + beta * num_nonp)
      * tgamma(4 + beta * num_ep) / tgamma(beta * num_ep) // eps node
      // from eps node
      * tgamma(beta * eta * num_ep) / tgamma(4 + beta * eta * num_ep)
      * tgamma(4 + beta * eta) / tgamma(beta * eta) // 0 of Ep(0,1)
      * tgamma(0 + beta * eta) / tgamma(beta * eta); // 1 of Ep(0,1)
    norm(true_probs);

    TS_ASSERT_DELTA(probs[0], true_probs[0], delta);
    TS_ASSERT_DELTA(probs[1], true_probs[1], delta);

    // dtree probs when z = 1
    lda.calc_dtree_probs(1, probs);

    // true prob when z = 1 and dnf = ";0,1"
    num_np = 2;
    num_nonp = 1;
    num_ep = 0;
    true_probs[0] = 1
      // for root node
      * tgamma(beta * eta * num_nonp + beta * num_np) / tgamma(8 + beta * eta * num_nonp + beta * num_np)
      * tgamma(0 + beta * eta * num_nonp) / tgamma(beta * eta * num_nonp) // non-np node
      * tgamma(4 + beta) / tgamma(beta) // Np(0)
      * tgamma(4 + beta) / tgamma(beta) // Np(1)
      // for non-np node
      * tgamma(beta * num_nonp) / tgamma(0 + beta * num_nonp)
      * tgamma(0 + beta) / tgamma(beta); // normal leaf 2

    // true prob when z = 1 and dnf = "0,1;2"
    num_np = 1;
    num_nonp = 2;
    num_ep = 2;
    true_probs[1] = 2
      // for root node
      * tgamma(beta * eta * num_nonp + beta * num_np) / tgamma(8 + beta * eta * num_nonp + beta * num_np)
      * tgamma(8 + beta * eta * num_nonp) / tgamma(beta * eta * num_nonp) // non-np node
      * tgamma(0 + beta) / tgamma(beta) // Np(2)
      // for non-np node
      * tgamma(beta * num_nonp) / tgamma(8 + beta * num_nonp)
      * tgamma(8 + beta * num_ep) / tgamma(beta * num_ep) // eps node
      // for eps node
      * tgamma(beta * eta * num_ep) / tgamma(8 + beta * eta * num_ep)
      * tgamma(4 + beta * eta) / tgamma(beta * eta) // 0 of Ep(0,1)
      * tgamma(4 + beta * eta) / tgamma(beta * eta); // 1 of Ep(0,1)
    norm(true_probs);

    TS_ASSERT_DELTA(probs[0], true_probs[0], delta);
    TS_ASSERT_DELTA(probs[1], true_probs[1], delta);
  }

  void test_calc_dtree_probs_real() {
    // make dnf file
    ofstream file(tmp_file.c_str());
    file << ";1,0\n0,1;2\n0,2;1\n;2,0"; // (ML(0,1) | ML(0,2)) & CL(1,2)
    file.close();

    lda.dnf_file = tmp_file;
    lda.initialize();

    // 1st sample of seed=92
    int sample_hz[4][4] = {
      {1, 0, 1, 1}, 
      {0, 1, 0, 0},
      {1, 1, 1, 1}, 
      {0, 1, 1, 1} 
    };

    // preprocess by sample_hz
    for(int d = 0; d < lda.num_docs; ++d) {
      for(int i = 0; i < lda.nd[d]; ++i) {
        int w = lda.docs[d][i];
        int z = sample_hz[d][i];
        lda.resample_post(d, w, z);
      }
    }

    // true probs with eta=10, alpha=0.01, beta=0.01, 
    double true_probs[2][4] = {
      {0.0764325, 0.0770787, 0.839471, 0.0070179},
      {0.0410388, 0.47415, 0.440653, 0.0441584}
    };

    vector<double> probs(lda.num_dtrees);
    for(int z = 0; z < lda.num_topics; ++z) {
      lda.calc_dtree_probs(z, probs);
      for(int t = 0; t < lda.num_dtrees; ++t) {
        TS_ASSERT_DELTA(probs[t], true_probs[z][t], delta);
      }
    }
  }

  void test_calc_prob_weight() {
    lda.initialize();
    lda.dz[0] = 0;
    lda.dz[1] = 1;

    // fixed sampling
    for(int i = 0; i < 4; ++i) {
      lda.resample_post(0, 0, 1);
      lda.resample_post(0, 0, 0);
      lda.resample_post(0, 1, 1);
      lda.resample_post(0, 2, 0);
    }

    double beta = lda.beta;
    double eta = lda.eta;
    int num_words = lda.num_words;
    int num_topics = lda.num_topics;
    int num_np, num_nonp, num_ep;

    // z = 0 -> dnf = ";0,1"
    num_np = 2;
    num_nonp = 1;
    num_ep = 0;
    TS_ASSERT_EQUALS(lda.calc_prob_weight(0, 0), // Np(0)
                     (4 + beta) / (8 + beta * eta * num_nonp + beta * num_np));
    TS_ASSERT_EQUALS(lda.calc_prob_weight(1, 0), // Np(1)
                     (0 + beta) / (8 + beta * eta * num_nonp + beta * num_np));
    TS_ASSERT_EQUALS(lda.calc_prob_weight(2, 0), // free
                     (4 + beta) / (4 + beta * num_nonp)
                     * (4 + beta * eta * num_nonp) / (8 + beta * eta * num_nonp + beta * num_np));

    // z = 1 -> dnf = "0,1;2"
    num_np = 1;
    num_nonp = 2;
    num_ep = 2;
    TS_ASSERT_EQUALS(lda.calc_prob_weight(0, 1), // Ep(0,1)
                     (4 + beta * eta) / (8 + beta * eta * num_ep)
                     * (8 + beta * num_ep) / (8 + beta * num_nonp)
                     * (8 + beta * eta * num_nonp) / (8 + beta * eta * num_nonp + beta * num_np));
    TS_ASSERT_EQUALS(lda.calc_prob_weight(1, 1), // Ep(0,1)
                     (4 + beta * eta) / (8 + beta * eta * num_ep)
                     * (8 + beta * num_ep) / (8 + beta * num_nonp)
                     * (8 + beta * eta * num_nonp) / (8 + beta * eta * num_nonp + beta * num_np));
    TS_ASSERT_EQUALS(lda.calc_prob_weight(2, 1), // Np(2)
                     (0 + beta) / (8 + beta * eta * num_nonp + beta * num_np));
  }
};
