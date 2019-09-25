import os
import sys

from dnf import DNF

class ParamViewer:
    def __init__(self, param_pref, lex_file, dnf_file, num_words, num_topics, num_docs, out_file, verbose):
        self.param_pref = param_pref
        self.lex_file = lex_file
        self.dnf_file = dnf_file
        self.num_words = num_words
        self.num_topics = num_topics
        self.num_docs = num_docs
        self.out_file = out_file
        self.verbose = verbose

        self.id2word = None
        if self.lex_file:
            self.id2word = self.load_lex(self.lex_file)

        self.dnf = None
        if self.dnf_file:
            self.dnf = DNF()
            self.dnf.load(dnf_file, self.id2word)

        self.out = sys.stdout
        if self.out_file:
            self.out = open(self.out_file, 'w')

    def __del__(self):
        if self.out_file:
            self.out.close()

    def show(self):
        self.show_phi(self.param_pref + '.phi')
        self.show_dti(self.param_pref + '.dti')
        if self.verbose:
            self.show_theta(self.param_pref + '.theta')
            self.show_smp(self.param_pref + '.smp')

    def show_phi(self, phi_file):
        if not os.path.exists(phi_file):
            print('Not found: {}'.format(phi_file), file=sys.stderr)
            return

        self.print_text('- Word probabilities in each topic')
        prob_mat = self.load_mat(phi_file)
        prob_mat = self.transpose(prob_mat)
        for tid, probs in enumerate(prob_mat):
            if tid >= self.num_topics:
                self.print_text('...')
                break
            self.print_text('<topic-{}>'.format(tid))
            for word, prob in sorted(enumerate(probs), key=lambda x: -x[1])[:self.num_words]:
                if self.id2word:
                    if word in self.id2word:
                        word = self.id2word[word]
                    else:
                        exit('Unknown word id: {}'.format(word))
                self.print_text('{}\t{:0.6f}'.format(word, prob))
            if len(probs) > self.num_words: self.print_text('...')
        self.print_text()

    def show_dti(self, dti_file):
        if not os.path.exists(dti_file):
            # print('Not found: {}'.format(dti_file), file=sys.stderr) # ignore for LDA
            return

        self.print_text('- Assignment of dtree on topics')
        if self.dnf:
            self.print_text('dtrees: {}'.format(self.dnf))
        dtrees = []
        with open(dti_file) as f:
            dtrees = map(int, f.read().split())
        for tid, dtree in enumerate(dtrees):
            if tid >= self.num_topics:
                self.print_text('...')
                break
            if self.dnf:
                if dtree < len(self.dnf):
                    dtree = self.dnf[dtree]
                else:
                    exit('Unknown dtree id: {}'.format(dtree))
            self.print_text('topic-{}: {}'.format(tid, dtree))
        self.print_text()

    def show_smp(self, smp_file):
        if not os.path.exists(smp_file):
            print('Not found: {}'.format(smp_file), file=sys.stderr)
            return

        self.print_text('- Word-topic counts (c_{wz}) of the last sample')
        if self.id2word:
            words = list(self.id2word.values())
            if len(words) > self.num_words:
                words = words[:self.num_words]
                words.append('...')
            self.print_text('         {}'.format('\t'.join(words)))
                
        for tid, line in enumerate(open(smp_file)):
            if tid >= self.num_topics:
                self.print_text('...')
                break
            counts = line.strip().split()
            self.print_text('topic-{}: {}'.format(tid, '\t'.join(counts[:self.num_words])))
        self.print_text()

    def show_theta(self, theta_file):
        if not os.path.exists(theta_file):
            print('Not found: {}'.format(theta_file), file=sys.stderr)
            return

        self.print_text('- Topic probabilities in each document')
        prob_mat = self.load_mat(theta_file)
        for did, probs in enumerate(prob_mat):
            if did >= self.num_docs:
                self.print_text('...')
                break
            self.print_text('<doc-{}>'.format(did))
            for tid, prob in sorted(enumerate(probs), key=lambda x: -x[1])[:self.num_topics]:
                self.print_text('topic-{}\t{:0.6f}'.format(tid, prob))
            if len(probs) > self.num_topics: self.print_text('...')
        self.print_text()

    def load_lex(self, lex_file):
        id2word = {}
        with open(lex_file) as f:
            for i, line in enumerate(f):
                word = line.strip()
                id2word[i] = word
        return id2word

    def load_mat(self, mat_file):
        prob_mat = []
        with open(mat_file) as f:
            for line in f:
                probs = map(float, line.strip().split())
                prob_mat.append(list(probs))
        return prob_mat

    def transpose(self, mat):
        return [[row[i] for row in mat] for i in range(len(mat[0]))]

    def print_text(self, text=''):
        print(text, file=self.out)


if __name__ == '__main__':
    import argparse

    desc = """
Viewer of learned parameters

examples:
python utils/%(prog)s out/test.final
python utils/%(prog)s -p out/test.final -l data/test.lex -d data/test.dnf -n 1"""
    arg_parser = argparse.ArgumentParser(description=desc, formatter_class=argparse.RawDescriptionHelpFormatter)
    arg_parser.add_argument('-p', '--param', metavar='PREF',
                            help='prefix of path to learned parameters (.phi, .dti, .smp)')
    arg_parser.add_argument('-l', '--lex', metavar='FILE',
                            help='file (.lex) for lexicons in .dat file')
    arg_parser.add_argument('-d', '--dnf', metavar='FILE',
                            help='file (.dnf) for compliled dnf of primitives')
    arg_parser.add_argument('-n', '--num_words', metavar='N', type=int, default=10,
                            help='maximum number of words to be displayed')
    arg_parser.add_argument('-t', '--num_topics', metavar='N', type=int, default=10,
                            help='maximum number of topics to be displayed')
    arg_parser.add_argument('-m', '--num_docs', metavar='N', type=int, default=10,
                            help='maximum number of docs to be displayed')
    arg_parser.add_argument('-o', '--out', metavar='FILE',
                            help='file (.fot) to save')
    arg_parser.add_argument('-v', '--verbose', action='store_true',
                            help='verbose mode')

    args = arg_parser.parse_args()
    if not (args.param):
        arg_parser.print_help()
        exit()

    viewer = ParamViewer(param_pref=args.param, lex_file=args.lex, dnf_file=args.dnf,
                         num_words=args.num_words, num_topics=args.num_topics, num_docs=args.num_docs,
                         out_file=args.out, verbose=args.verbose)
    viewer.show()
