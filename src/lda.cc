#include "lda.h"

#include <cassert>
#include <cmath>
#include <cstdlib>

#include <fstream>
#include <iostream>
using namespace std;

#include "utils.h"
using namespace ldautils;


LDA::LDA(string data_file_, string out_base_, int num_topics_, double alpha_, double beta_,
         int max_steps_, int num_loops_, int burn_in_, bool converge_, int rand_seed_, bool verbose_)
  :data_file(data_file_),
   out_base(out_base_),
   num_topics(num_topics_),
   alpha(alpha_),
   beta(beta_),
   max_steps(max_steps_),
   num_loops(num_loops_),
   burn_in(burn_in_),
   converge(converge_),
   rand_seed(rand_seed_),
   verbose(verbose_) {

  assert(num_topics > 0);
  assert(alpha > 0.0);
  assert(beta > 0.0);
  assert(max_steps > 0);

  set_verbose(verbose);
  if(out_base == "") {
    out_base = data_file;
  }

  comment("* Parameters");
  comment("- data file: " + data_file);
  comment("- out base: " + out_base);
  comment("- num topics: " + str(num_topics));
  comment("- alpha: " + str(alpha));
  comment("- beta: " + str(beta));
  comment("- max steps: " + str(max_steps));
  comment("- num loops: " + str(num_loops));
  comment("- burn in: " + str(burn_in));
  comment("- converge: " + str(converge));
  comment("- rand seed: " + str(rand_seed));
  comment("- verbose: " + str(verbose));
}

void
LDA::run() {
  initialize();
  preprocess();
  infer();
}

void
LDA::initialize() {
  comment("* Initialization");
  comment("- loading " + data_file);
  load_data(data_file);
  
  comment("# docs: " + str(num_docs));
  comment("# words: " + str(num_words));
  comment("# terms: " + str(num_terms));

  srand(static_cast<unsigned>(rand_seed));

  cz.assign(num_topics, 0);
  cdz.assign(num_docs, cz);
  cwz.assign(num_words, cz);
  hz.assign(num_docs, vector<int>());
  for(int d = 0; d < num_docs; d++) {
    hz[d].assign(nd[d], 0);
  }

  alphas.assign(num_topics, alpha);
  betas.assign(num_words, beta);

  probs.assign(num_topics, 0.0);
  phi.assign(num_topics, vector<double>(num_words));
  theta.assign(num_docs, vector<double>(num_topics));
}

void
LDA::preprocess() {
  comment("* Preprocessing");
  probs.assign(num_topics, 1.0/num_topics);
  for(int d = 0; d < num_docs; d++) {
    for(int i = 0; i < nd[d]; i++) {
      int w = docs[d][i];
      int z = multi(probs);
      resample_post(d, w, z);
      hz[d][i] = z;
    }
  }
}

void
LDA::infer() {
  comment("* Inference");
  int step_every = max_steps / 10;
  double old_pp = -1.0;
  double converge_limit = 0.001;
  for(int i = 0; i < max_steps; i++) {
    resample();
    double pp = calc_perplexity();
    
    if(step_every == 0 || i % step_every == 0) {
      comment("- step " + str(i) + ": pp = " + str(pp));
      if(verbose) {
        save_params(out_base + ".step_" + str(i));
      }
    }

    if(i < burn_in) continue;

    if(converge && fabs(pp - old_pp) < converge_limit) {
      comment("- converged"); // (heuristic) local optima of sampling
      break;
    }
    old_pp = pp;
  
    for(int j = 0; j < num_loops; j++) {
      update_params();
    }
  }
  save_params(out_base + ".final");
  comment("* Finish");
}

void
LDA::load_data(const string &file_name) {
  ifstream in(file_name.c_str());
  if(!in.is_open()) {
    cerr << "LDA::load_data(): cannot open " << file_name << endl;
    exit(1);
  }

  string line;
  vector<int> doc;
  vector<string> wfs; // ("word:freq", "word2:freq2", ...)
  vector<string> wf; // ("word", "freq")
  int max_wid = 0;
  docs.clear();
  nd.clear();
  while(getline(in, line)) {
    split(line, ' ', wfs);
    doc.clear();
    for(vector<string>::iterator s = wfs.begin(); s != wfs.end(); ++s) {
      if(*s == "") continue;
      split(*s, ':', wf);
      assert(wf.size() == 2);
      int wid = atoi(wf[0].c_str());
      int freq = atoi(wf[1].c_str());
      for(int j = 0; j < freq; ++j) {
        doc.push_back(wid);
      }
      if(max_wid < wid) {
        max_wid = wid;
      }
    }
    nd.push_back(doc.size());
    docs.push_back(doc);
  }
  num_docs = docs.size();
  num_words = max_wid + 1; // not assuming missing words
  num_terms = sum(nd);
}

void
LDA::resample() { 
  for(int d = 0; d < num_docs; d++) {
    for(int i = 0; i < nd[d]; i++) {
      int w = docs[d][i];
      int z = hz[d][i];

      resample_pre(d, w, z);

      calc_probs(d, w, probs);
      z = multi(probs);

      resample_post(d, w, z);
      hz[d][i] = z;
    }
  }
}

void
LDA::resample_pre(int d, int w, int z) {
  --cdz[d][z];
  --cwz[w][z];
  --cz[z];
}

void
LDA::resample_post(int d, int w, int z) {
  ++cdz[d][z];
  ++cwz[w][z];
  ++cz[z];
}

void
LDA::calc_probs(int d, int w, vector<double> &probs) {
  assert(probs.size() == num_topics);
  for(int j = 0; j < num_topics; ++j) {
    probs[j] = (cwz[w][j] + betas[w]) * (cdz[d][j] + alphas[j]);
    double denom = cz[j] + beta * num_words;
    if(denom > 0) probs[j] /= denom;
    else cerr << "warning in LDA::calc_probs(): denom is zero" << endl;
  }
  norm(probs);
}

void
LDA::update_params() {
  // hyperparameter update by Minka's fixed point iteration
  // cf. https://tminka.github.io/papers/dirichlet/minka-dirichlet.pdf
  double sum_alpha = sum(alphas);
  double min_value = 0.00001;
  for(int z = 0; z < num_topics; z++) {
    double num = 0;
    double denom = 0;
    for(int d = 0; d < num_docs; d++) {
      num += digamma(cdz[d][z]+alphas[z]) - digamma(alphas[z]);
      denom += digamma(nd[d]+sum_alpha) - digamma(sum_alpha);
    }
    if(num <= 0 || denom <= 0) {
      //cerr << "warning in LDA::update_params(): invalid update" << endl;
      num += min_value;
      denom += min_value;
    }
    alphas[z] = alphas[z] * num / denom;
    if(alphas[z] < min_value) {
      alphas[z] = min_value;
    }
  }
}

double
LDA::calc_perplexity() {
  get_theta(theta);
  get_phi(phi);
  
  double lik = 0.0;
  for(int d = 0; d < num_docs; d++) {
    for(int i = 0; i < nd[d]; i++) {
      int w = docs[d][i];
      double prob = 0.0;
      for(int z = 0; z < num_topics; z++) {
        prob += theta[d][z]*phi[z][w];
      }
      assert(prob > 0);
      lik += log(prob);
    }
  }
  return exp(-lik/num_terms);
}

void
LDA::save_params(const string &out_base) {
  comment("wrote to " + out_base + ".*");

  string theta_file = out_base + ".theta";
  get_theta(theta);
  save_matrix(theta_file, theta);

  string phi_file = out_base + ".phi";;
  get_phi(phi);
  save_matrix_t(phi_file, phi);

  string smp_file = out_base + ".smp";
  save_matrix_t(smp_file, cwz);
}

void
LDA::get_phi(vector<vector<double> > &phi) {
  assert(phi.size() == num_topics);
  assert(phi[0].size() == num_words);
  for(int z = 0; z < num_topics; z++) {
    for(int w = 0; w < num_words; w++) {
      phi[z][w] = cwz[w][z] + betas[w];
    }
    norm(phi[z]);
  }
}

void
LDA::get_theta(vector<vector<double> > &theta) {
  assert(theta.size() == num_docs);
  assert(theta[0].size() == num_topics);
  for(int d = 0; d < num_docs; d++) {
    for(int z = 0; z < num_topics; z++) {
      theta[d][z] = cdz[d][z] + alphas[z];
    }
    norm(theta[d]);
  }
}

void
LDA::print_debug() {
  cout << "cdz:" << endl;
  for(int d = 0; d < num_docs; ++d) {
    for(int z = 0; z < num_topics; ++z) {
      cout << cdz[d][z] << " ";
    }
    cout << endl;
  }
  cout << "cwz:" << endl;
  for(int w = 0; w < num_words; ++w) {
    for(int z = 0; z < num_topics; ++z) {
      cout << cwz[w][z] << " ";
    }
    cout << endl;
  }
  cout << "cz:" << endl;
  for(int z = 0; z < num_topics; ++z) {
    cout << cz[z] << " ";
  }
  cout << endl;
  cout << "hz:" << endl;
  for(int d = 0; d < num_docs; ++d) {
    for(int i = 0; i < nd[d]; ++i) {
      cout << hz[d][i] << " ";
    }
    cout << endl;
  }
}
