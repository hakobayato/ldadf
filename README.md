# Topic Modeling with Logical Constraints on Words

This is a re-implementation of my work ["Topic Models with Logical Constraints on Words" (ROBUS 2011)](https://www.aclweb.org/anthology/W11-3905), which allows us to use any logical expressions of soft constraints on words, such as Must-Link (ML) and Cannot-Link (CL) defined below when analyzing documents by topic modeling.
- ML(A, B): A and B must appear in the same topic.
- CL(A, B): A and B cannot apper in the same topic.

For example, we can use this soft constraint, (ML("kung-fu", "jackie") | ML("kung-fu", "bruce")) & CL("bruce", "jackie"), to distinguish Jackie Chan and Bruce Lee when analyzing movie reviews. The proposed model involves a latent Dirichlet allocation with Dirichlet forest priors (LDA-DF) trained via collapsed Gibbs sampling. Note that this implementation is a simplified version of the original LDA-DF, where this one directly encodes the maximal independent sets on a CL-graph into trees (see the [implementation notes](docs/ldadf_notes.pdf)), whereas the original one encodes the cliques on each connected component in its complement graph into subtrees. See the [paper](https://www.aclweb.org/anthology/W11-3905) or [slides](http://hayatokobayashi.com/slides/ROBUS2011_Kobayashi_slides.pdf) for details.

## Requirements
- GCC (4.8.5)
- Python (3.6.6)
- PLY (3.10)
- CxxTest (4.3)

## Installation
PLY (Python Lex-Yacc) is used to parse logical expressions.
```
$ pip install ply
```

## Example
The following command runs an interactive demonstration when analyzing a toy dataset ([[A,A,B,B], [A,A,C,C], [A,A,B,B], [A,A,C,C]]) with two topics.
```
$ python example.py -I

Select a constraint:
...
6) ( ML("A", "B") | ML("A", "C") ) & CL("B", "C")
...

Index> 6
...
- Word probabilities in each topic
<topic-0>
A       0.499500
C       0.499500
B       0.000999
<topic-1>
A       0.499500
B       0.499500
C       0.000999
```

## Usage

### example.py
Example script to understand how to use this software.
```
usage: example.py [-h] [-i N] [-e N] [-s N] [-v] [-I]

Demonstration of topic modeling with logical constraints

examples:
python example.py -I
python example.py -i 6 -e 100 -s 0 -v

prepared constraint links:
0) 
1) ML("A","B")
2) CL("B", "C")
3) IL("B", "A")
4) ML("A", "B") | ML("A", "C")
5) IL("B", "A") & IL("C", "A")
6) ( ML("A", "B") | ML("A", "C") ) & CL("B", "C")
7) IL("B", "A") & IL("C", "A") & CL("B", "C")
8) ML("A", "B") & ML("A", "C")
9) ML("A", "B") & CL("B", "C")

optional arguments:
  -h, --help         show this help message and exit
  -i N, --index N    index of prepared constraint links
  -e N, --eta N      eta parameter for strength of links
  -s N, --seed N     eta parameter for strength of links
  -v, --verbose      verbose mode
  -I, --interactive  interactive mode
```

### utils/make-data.py
Script to make a training dataset (.dat/lex) from a raw input file including space-separated word sequences. The .dat file includes data sequences of word-ids, and the .lex file is its dictionary.
```
usage: make_data.py [-h] [-i FILE] [-o PREF] [-m N]

Script to make a training dataset (.dat/lex) from a raw text file (.txt)

examples:
python utils/make_data.py -i data/test.txt -o out/test -m 1

optional arguments:
  -h, --help            show this help message and exit
  -i FILE, --input FILE
                        input file (.txt) for raw texts
  -o PREF, --out_base PREF
                        output prefix of path to save .dat/lex files
  -m N, --min_freq N    minimum frequency for .lex file
```
We can make a dataset as follows.
```
$ cat data/test.txt
A A B B
A A C C
A A B B
A A C C
$ python utils/make_data.py -i data/test.txt -o out/test -m 1
* Info:
- raw_file: data/test.txt
- lex_file: out/test.lex
- dat_file: out/test.dat
- min_freq: 1
* Making lex file
* Making dat file
* Done
$ cat out/test.dat out/test.lex
0:2 1:2
0:2 2:2
0:2 1:2
0:2 2:2
$ cat out/test.lex
A
B
C
```

### utils/compiler.py
Compiler to convert a given constraint (logical expressions of MLs and CLs) into the corresponding DNF of primitives.
```
usage: compiler.py [-h] [-l FILE] [-c FILE] [-d FILE] [-i] [--debug] [-v]

Compiler from constraint linkes to DNF of primitives

example:
python utils/compiler.py -c data/test.cst -d data/test.dnf -l data/test.lex -v
python utils/compiler.py -I
    

optional arguments:
  -h, --help           show this help message and exit
  -l FILE, --lex FILE  lexicon file (.lex) for .dat file
  -c FILE, --cst FILE  file (.cst) to load constraint links
  -d FILE, --dnf FILE  file (.dnf) to save compiled DNF
  -I, --interactive    interactive mode for debugging used without any other
                       options
  --debug              debug mode for ply parser
  -v, --verbose        verbose mode
```
We can create .dnf file from a text or file including a constraint as follows.
```
$ python utils/compiler.py -v -c '(ML("A","B")|ML("A","C"))&CL("B","C")' -l./data/test.lex -d out/test.dnf
compiled DNF: Ep(0,1)&Np(2) | Ep(0,2)&Np(1) | Np(0)&Np(1) | Np(0)&Np(2)
dtree-0: Np(0)&Np(1)
dtree-1: Ep(0,1)&Np(2)
dtree-2: Ep(0,2)&Np(1)
dtree-3: Np(0)&Np(2)
$ cat out/test.dnf 
;0,1
0,1;2
0,2;1
;0,2
```
Or, we can interactively check the functionality as follows.
```
$ python utils/compiler.py -I
Input links> (ML("A","B")|ML("A","C"))&CL("B","C")
dnf primitives: Ep(A,B)&Np(C) | Ep(A,C)&Np(B) | Np(A)&Np(B) | Np(A)&Np(C)
dnf file:
;A,B
A,B;C
A,C;B
;A,C
```

### src/ldadf
Main program to infer model parameters from a dataset and a compiled DNF.
```
usage: ldadf [OPTION..] DATA

LDA with logical constraints on words

examples:
./src/ldadf -n2 -m100 -o out/test -v data/test.dat
./src/ldadf -n2 -m100 -o out/test -v -d data/test.dnf -e10 data/test.dat

optional arguments
  -o    output path (prefix for .phi/.theta/.dti/.smp)
  -n    number of topics
  -a    hyperparameter alpha of document-topic distribution theta
  -b    hyperparameter beta of topic-word distribution phi
  -m    maximum number of training steps
  -l    number of inner loops
  -u    number of burn-in steps
  -c    stop training if perplexity does not change
  -s    seed of random function
  -v    verbose mode
  -d    file (.dnf) including compiled dnf from constraint linkes
  -e    strength parameter eta of constraint links
  -h    print this message
```
We can run this program as follows.
```
$ cd src; make release; cd ..
$ ./src/ldadf -n2 -m100 -o out/test -v -d data/test.dnf -e10 data/test.dat
* Parameters
- data file: data/test.dat
- out base: out/test
- num topics: 2
...
* Initialization
- loading data/test.dat
# docs: 4
# words: 3
# terms: 16
- loading data/test.dnf
- tree: Np(0,1)
- tree: Ep(0,1)^Np(2)
# dtrees: 2
* Preprocessing
* Inference
- step 0: pp = 2.71123
wrote to out/test.step_0.*
...
- step 90: pp = 2.50231
wrote to out/test.step_90.*
wrote to out/test.final.*
* Finish
```
### utils/viewer.py
Viewer to check the learned parameters
```
$ python utils/viewer.py
usage: viewer.py [-h] [-p PREF] [-l FILE] [-d FILE] [-n N] [-t N] [-m N]
                 [-o FILE] [-v]

Viewer of learned parameters

examples:
python utils/viewer.py out/test.final
python utils/viewer.py -p out/test.final -l data/test.lex -d data/test.dnf -n 1

optional arguments:
  -h, --help            show this help message and exit
  -p PREF, --param PREF
                        prefix of path to learned parameters (.phi, .dti,
                        .smp)
  -l FILE, --lex FILE   file (.lex) for lexicons in .dat file
  -d FILE, --dnf FILE   file (.dnf) for compliled dnf of primitives
  -n N, --num_words N   maximum number of words to be displayed
  -t N, --num_topics N  maximum number of topics to be displayed
  -m N, --num_docs N    maximum number of docs to be displayed
  -o FILE, --out FILE   file (.fot) to save
  -v, --verbose         verbose mode
```
We can see the topic-word probabilities, the assignment of Dirichlet trees on topics, and so on, as follows.
```
$ python utils/viewer.py -p out/test.final -l data/test.lex -d data/test.dnf -v
- Word probabilities in each topic
<topic-0>
A       0.663391
B       0.335790
C       0.000819
<topic-1>
C       0.995146
A       0.002427
B       0.002427

- Assignment of dtrees on topics
dtrees: Ep(A,B)&Np(C) | Np(A)&Np(B)
topic-0: Ep(A,B)&Np(C)
topic-1: Np(A)&Np(B)

- Topic probabilities in each document
<doc-0>
topic-0 0.870141
topic-1 0.129859
<doc-1>
topic-0 0.635590
topic-1 0.364410
<doc-2>
topic-0 0.870141
topic-1 0.129859
<doc-3>
topic-0 0.635590
topic-1 0.364410

- Word-topic counts (c_{wz}) of the last sample
         A      B       C
topic-0: 8      4       0
topic-1: 0      0       4
```

## Data Format
### .dat
This file includes data sequences in a dataset. One line represents one document expressed as a space-separated sequence of colon-separated (word-id, freq) pairs. The following example is the above-mentioned toy dataset ([[A,A,B,B], [A,A,C,C], [A,A,B,B], [A,A,C,C]]).
```
0:2 1:2 
0:2 2:2 
0:2 1:2 
0:2 2:2 
```
### .lex
This file includes the dictionary of .dat file. One line represents one word, which corresponds to the line number as word-id.
```
A
B
C
```
### .dnf
This file includes a compiled constraint (DNF). One line represents one conjunction that consists of semicolon-separated Eps and Np. Eps are separated with colons, and words are separated with commas. The following example expresses the DNF, Np(A,B) | Ep(A,B) & Np(C), compiled from a constraint, ML(A,B) & CL(B,C).
```
;0,1
0,1;2
```

## Citation
```
@inproceedings{kobayashi-etal-2011-topic,
    title = "Topic Models with Logical Constraints on Words",
    author = "Kobayashi, Hayato and Wakaki, Hiromi and Yamasaki, Tomohiro and Suzuki, Masaru",
    booktitle = "Proceedings of Workshop on Robust Unsupervised and Semisupervised Methods in Natural Language Processing",
    year = "2011",
    publisher = "Association for Computational Linguistics",
    url = "https://www.aclweb.org/anthology/W11-3905",
    pages = "33--40",
}
```

## License
This software is released under the MIT License. See [LICENSE](LICENSE).
