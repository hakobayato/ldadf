#include "ldadf.h"

#include <cassert>
#include <cmath>
#include <cstdlib>

#include <iostream>
#include <fstream>
#include <numeric>
using namespace std;

#include "utils.h"
using namespace ldautils;

LDADF::LDADF(string datafile_, string out_base_, int num_topics_, double alpha_, double beta_,
             int max_steps_, int num_loops_, int burn_in_, bool converge_, int rand_seed_, bool verbose_,
             string dnf_file_, double eta_)
  : LDA(datafile_, out_base_, num_topics_, alpha_, beta_,
	max_steps_, num_loops_, burn_in_, converge_, rand_seed_, verbose_),
    dnf_file(dnf_file_),
    eta(eta_) {

  comment("- dnf file: " + dnf_file);
  comment("- eta: " + str(eta));
}

void
LDADF::initialize() {
  LDA::initialize();

  comment("- loading " + dnf_file);

  load_dnf(dnf_file);
  comment("# dtrees: " + str(num_dtrees));

  ctz.assign(num_dtrees, vector<int>(num_topics, 0));
  for(int t = 0; t < num_dtrees; ++t) {
    ctze.push_back(vector<vector<int> >(num_topics, vector<int>(dtrees[t].eps.size(), 0)));
  }

  dz.assign(num_topics, 0);
  dtree_probs.assign(num_dtrees, 0.0);
}

void
LDADF::preprocess() {
  // tree sampling
  for(int t = 0; t < num_dtrees; t++) {
    dtree_probs[t] = num_words - dtrees[t].np.size();
  }
  norm(dtree_probs);
  for(int z = 0; z < num_topics; ++z) {
    dz[z] = multi(dtree_probs);
  }

  // topic sampling
  LDA::preprocess();
}

void
LDADF::resample() {
  // tree sampling
  for(int z = 0; z < num_topics; ++z) {
    calc_dtree_probs(z, dtree_probs);
    dz[z] = multi(dtree_probs);
  }

  // topic sampling
  LDA::resample();
}

void
LDADF::resample_pre(int d, int w, int z) {
  LDA::resample_pre(d, w, z);

  for(int t = 0; t < num_dtrees; ++t) {
    DTree &dt = dtrees[t];
    int e;
    switch(dt.get_type(w)) {
    case DTree::Ep:
      e = dt.get_ep(w);
      --ctz[t][z];
      --ctze[t][z][e];
      break;
    case DTree::None:
      --ctz[t][z];
      break;
    }
  }
}

void
LDADF::resample_post(int d, int w, int z) {
  LDA::resample_post(d, w, z);

  for(int t = 0; t < num_dtrees; ++t) {
    DTree &dt = dtrees[t];
    int e;
    switch(dt.get_type(w)) {
    case DTree::Ep:
      e = dt.get_ep(w);
      ++ctz[t][z];
      ++ctze[t][z][e];
      break;
    case DTree::None:
      ++ctz[t][z];
      break;
    }
  }
}

void
LDADF::calc_probs(int d, int w, vector<double> &probs) {
  for(int z = 0; z < num_topics; ++z) {
    probs[z] = (cdz[d][z] + alphas[z]);
    probs[z] *= calc_prob_weight(w, z);
  }
  norm(probs);
}

void
LDADF::get_phi(vector<vector<double> > &phi) {
  for(int z = 0; z < num_topics; ++z) {
    for(int w = 0; w < num_words; ++w) {
      phi[z][w] = calc_prob_weight(w, z);
    }
    norm(phi[z]);
  }
}

void
LDADF::save_params(const string &out_base) {
  LDA::save_params(out_base);

  string dti_file = out_base + ".dti";
  vector<vector<int> > mat(1, dz);
  save_matrix(dti_file, mat);
}

void
LDADF::load_dnf(const string &filename) {
  ifstream in(filename.c_str());
  if(!in.is_open()) {
    cerr << "LDADF::load_dnf(): cannot open " << filename << endl;
    exit(1);
  }

  string line;
  while(getline(in, line)) {
    DTree dtree;
    dtree.parse(line);
    comment("- tree: " + dtree.str());
    dtrees.push_back(dtree);
  }
  num_dtrees = dtrees.size();
}

void
LDADF::calc_dtree_probs(int z, vector<double> &dtree_probs) {
  assert(dtree_probs.size() == num_dtrees);
  for(int t = 0; t < num_dtrees; ++t) {
    dtree_probs[t] = calc_dtree_prob_weight(z, t);
  }

  // logsumexp trick
  double maxval = max(dtree_probs);
  for(int t = 0; t < num_dtrees; ++t) {
    dtree_probs[t] = exp(dtree_probs[t] - maxval);
  }
  norm(dtree_probs);
}

double
LDADF::calc_dtree_prob_weight(int z, int t) {
  DTree &dt = dtrees[t];
  int num_np = dt.np.size();
  int num_nonp = num_words - num_np;

  // size
  double prob = log(num_nonp);

  // root node
  prob += lgamma(beta * eta * num_nonp + beta * num_np) - lgamma(cz[z] + beta * eta * num_nonp + beta * num_np);
  prob += (lgamma(ctz[t][z] + beta * eta * num_nonp) - lgamma(beta * eta * num_nonp));
  for(int j = 0; j < num_np; ++j) {
    int w = dt.np[j];
    prob += lgamma(cwz[w][z] + beta) - lgamma(beta);
  }

  // non-np node
  prob += lgamma(beta * num_nonp) - lgamma(ctz[t][z] + beta * num_nonp);
  for(int w = 0; w < num_words; ++w) {
    if(dt.get_type(w) == DTree::None) {
      prob += lgamma(cwz[w][z] + beta) - lgamma(beta);
    }
  }
  int num_eps = dt.eps.size();
  for(int e = 0; e < num_eps; ++e) {
    int num_ep = dt.eps[e].size();
    prob += lgamma(ctze[t][z][e] + beta * num_ep) - lgamma(beta * num_ep);
  }

  // eps node
  for(int e = 0; e < num_eps; ++e) {
    int num_ep = dt.eps[e].size();
    prob += lgamma(beta * eta * num_ep) - lgamma(ctze[t][z][e] + beta * eta * num_ep);
    for(int i = 0; i < num_ep; ++i) {
      int w = dt.eps[e][i];
      prob += lgamma(cwz[w][z] + beta * eta) - lgamma(beta * eta);
    }
  }

  return prob;
}

double
LDADF::calc_prob_weight(int w, int z) {
  int t = dz[z];
  DTree &dt = dtrees[t];
  int num_np = dt.np.size();
  int num_nonp = num_words - num_np;

  double prob;
  int e, num_ep;
  switch(dt.get_type(w)) {
  case DTree::Ep:
    e = dt.get_ep(w);
    num_ep = dt.eps[e].size();
    prob = (cwz[w][z] + beta * eta);
    prob /= (ctze[t][z][e] + beta * eta * num_ep);
    prob *= (ctze[t][z][e] + beta * num_ep);
    prob /= (ctz[t][z] + beta * num_nonp);
    prob *= (ctz[t][z] + beta * eta * num_nonp);
    prob /= (cz[z] + beta * eta * num_nonp + beta * num_np);      
    break;
  case DTree::Np:
    prob = (cwz[w][z] + beta);
    prob /= (cz[z] + beta * eta * num_nonp + beta * num_np);
    break;
  case DTree::None:
    prob = (cwz[w][z] + beta);
    prob /= (ctz[t][z] + beta * num_nonp);
    prob *= (ctz[t][z] + beta * eta * num_nonp);
    prob /= (cz[z] + beta * eta * num_nonp + beta * num_np);
    break;
  }

  return prob;
}
