#include "ldadf.h"

#include <cstdlib>
#include <ctime>

#include <iostream>
using namespace std;

#include <getopt.h>

int
main(int argc, char *argv[]) {
  string out_base = "";
  int num_topics = 10;
  double alpha = 1.0;
  double beta = 0.01;
  int max_steps = 10;
  int num_loops = 10;
  int burn_in = 5;
  bool converge = false;
  int seed = time(0);
  bool verbose = false;
  string dnf_file = "";
  double eta = 10;
  bool help = false;

  int result;
  while((result=getopt(argc, argv, "o:n:a:b:m:l:u:cs:vd:e:h")) != -1){
    switch(result){
    case 'o':
      out_base = optarg;
      break;
    case 'n':
      num_topics = atoi(optarg);
      break;
    case 'a':
      alpha = atof(optarg);
      break;
    case 'b':
      beta = atof(optarg);
      break;
    case 'm':
      max_steps = atoi(optarg);
      break;
    case 'l':
      num_loops = atoi(optarg);
      break;
    case 'u':
      burn_in = atoi(optarg);
      break;
    case 'c':
      converge = true;
      break;
    case 's':
      seed = atoi(optarg);
      break;
    case 'v':
      verbose = true;
      break;
    case 'd':
      dnf_file = optarg;
      break;
    case 'e':
      eta = atof(optarg);
      break;
    case 'h':
      help = true;
      break;
    }
  }

  vector<string> args(&(argv[optind]), &(argv[argc]));
  if(args.size() == 0 || help == true) {
    cerr << "usage: ldadf [OPTION..] DATA" << endl;
    cerr << endl;
    cerr << "LDA with logical constraints on words" << endl;
    cerr << endl;
    cerr << "examples:" << endl;
    cerr << "./src/ldadf -n2 -m100 -o out/test -v data/test.dat" << endl;
    cerr << "./src/ldadf -n2 -m100 -o out/test -v -d data/test.dnf -e10 data/test.dat" << endl;
    cerr << endl;
    cerr << "optional arguments" << endl;
    cerr << "  -o    output path (prefix for .phi/.theta/.dti/.smp)" << endl;
    cerr << "  -n    number of topics" << endl;
    cerr << "  -a    hyperparameter alpha of document-topic distribution theta" << endl;
    cerr << "  -b    hyperparameter beta of topic-word distribution phi" << endl;
    cerr << "  -m    maximum number of training steps" << endl;
    cerr << "  -l    number of inner loops" << endl;
    cerr << "  -u    number of burn-in steps" << endl;
    cerr << "  -c    stop training if perplexity does not change" << endl;
    cerr << "  -s    seed of random function" << endl;
    cerr << "  -v    verbose mode" << endl;
    cerr << "  -d    file (.dnf) including compiled dnf from constraint linkes" << endl;
    cerr << "  -e    strength parameter eta of constraint links" << endl;
    cerr << "  -h    print this message" << endl;
    return 1;
  }

  string data = args[0];

  if(dnf_file != "") {
    LDADF lda(data, out_base, num_topics, alpha, beta,
              max_steps, num_loops, burn_in, converge, seed, verbose,
              dnf_file, eta);
    lda.run();
  } else {
    LDA lda(data, out_base, num_topics, alpha, beta,
            max_steps, num_loops, burn_in, converge, seed, verbose);
    lda.run();
  }

  return 0;
}
