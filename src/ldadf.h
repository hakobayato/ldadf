#ifndef LDADF_H
#define LDADF_H

#include <string>
#include <vector>

#include "lda.h"
#include "dtree.h"

class LDADF : public LDA {
  friend class TestLDADF;

 public:
  LDADF() {};
  LDADF(std::string datafile, std::string out_base = "", int num_topics = 10, double alpha = 0.1, double beta = 0.1,
        int max_steps = 100, int num_loops = 0, int burn_in = 5, bool converge = false, int rand_seed = 0, bool verbose = false,
        std::string dnf_file = "", double eta = 100);

  virtual void initialize();
  virtual void preprocess();

 protected:
  virtual void resample();
  virtual void resample_pre(int d, int w, int z);
  virtual void resample_post(int d, int w, int z);
  virtual void calc_probs(int d, int w, std::vector<double> &probs);

  virtual void get_phi(std::vector<std::vector<double> > &phi);
  virtual void save_params(const std::string &out_base);

  virtual void load_dnf(const std::string &filename);
  virtual void calc_dtree_probs(int z, std::vector<double> &dtree_probs);
  virtual double calc_dtree_prob_weight(int z, int t);
  virtual double calc_prob_weight(int w, int z);

 protected:
  // arguments
  std::string dnf_file;
  double eta;

  // dforest
  int num_dtrees;
  std::vector<DTree> dtrees;

  // counts for inference
  std::vector<int> dz; // dz[z] = index of dtree assigned for topic z
  std::vector<std::vector<int> > ctz; // ctz[t][z] = count of topic z for non-np words in dtree t
  std::vector<std::vector<std::vector<int> > > ctze; // ceps[t][z][e] = count of topic z for words in e-th ep in dtree t

  // temporary memory
  std::vector<double> dtree_probs;
};

#endif
