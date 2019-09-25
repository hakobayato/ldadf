#ifndef LDA_H
#define LDA_H

#include <string>
#include <vector>

class LDA {
  friend class TestLDA;

 public:
  LDA() {};
  LDA(std::string data_file, std::string out_base = "", int num_topics = 10, double alpha = 0.1, double beta = 0.1,
      int max_steps = 100, int num_loops = 0, int burn_in = 5, bool converge = false, int rand_seed = 0, bool verbose = false);

  virtual void run();
  virtual void initialize();
  virtual void preprocess();
  virtual void infer();

 protected:
  virtual void load_data(const std::string &file_name);

  virtual void resample();
  virtual void resample_pre(int d, int w, int z);
  virtual void resample_post(int d, int w, int z);
  virtual void calc_probs(int d, int w, std::vector<double> &probs);
  virtual void update_params();

  virtual double calc_perplexity();
  virtual void save_params(const std::string &out_base);
  virtual void get_phi(std::vector<std::vector<double> > &phi);
  virtual void get_theta(std::vector<std::vector<double> > &theta);

  virtual void print_debug();
  
 protected:
  // arguments
  std::string data_file;
  std::string out_base;
  int num_topics;
  int rand_seed;
  int max_steps;
  int num_loops;
  int burn_in;
  double alpha;
  double beta;
  bool converge;
  bool verbose;

  // docs
  std::vector<std::vector<int> > docs;
  int num_docs;
  int num_words;
  int num_terms;

  // hyper-parameters (can be updated)
  std::vector<double> alphas;
  std::vector<double> betas;

  // counts for inference
  std::vector<int> nd; // nd[d] = number of terms in document d
  std::vector<std::vector<int> > hz; // hz[d][i] = topic assigned for i-th term in document d
  std::vector<int> cz; // cz[z] = count of topic z
  std::vector<std::vector<int> > cdz; // cdz[d][z] = count of topic z for document d
  std::vector<std::vector<int> > cwz; // cwz[w][z] = count of topic z for word w

  // tenporary memory
  std::vector<double> probs;
  std::vector<std::vector<double> > phi;
  std::vector<std::vector<double> > theta;
};

#endif
