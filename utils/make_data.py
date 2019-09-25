import re
import sys

from collections import Counter

class MakeData:
    def __init__(self, raw_file, out_base, min_freq = 10):
        self.raw_file = raw_file
        self.out_base = out_base
        self.lex_file = out_base + '.lex'
        self.dat_file = out_base + '.dat'
        self.min_freq = min_freq

        # Use simple English words, other than numbers and symbols,
        # such as "the", "e-mail", "etc.", ignoring contractions ("n't")
        self.re_word = re.compile(r'^[a-zA-Z][a-zA-Z\-\.]*$')
        
    def run(self):
        print('* Info:')
        print('- raw_file: {}'.format(self.raw_file))
        print('- lex_file: {}'.format(self.lex_file))
        print('- dat_file: {}'.format(self.dat_file))
        print('- min_freq: {}'.format(self.min_freq))
        print('* Making lex file')
        lex = self.make_lex(self.raw_file, self.lex_file)
        print('* Making dat file')
        self.make_dat(self.raw_file, self.dat_file, lex)
        print('* Done')

    def make_lex(self, raw_file, lex_file):
        words = []
        with open(raw_file) as f:
            words = f.read().split()
        lex = {}
        most_common = Counter(words).most_common()
        with open(lex_file, 'w') as f:
            wid = 0
            for word, freq in most_common:
                if freq < self.min_freq: break
                if not self.is_word(word): continue
                f.write(word+'\n')
                lex[word] = wid
                wid += 1
        return lex
    
    def is_word(self, word):
        if len(word) > 27: # cf. longest word in Shakespere's works
            # print('- Too long word: {}'.format(word), file=sys.stderr)
            return False
        m = self.re_word.match(word)
        if not m:
            # print('- Invalid word: {}'.format(word), file=sys.stderr)
            return False
        return True

    def make_dat(self, raw_file, dat_file, lex):
        with open(raw_file) as fin, open(dat_file, 'w') as fout:
            for line in fin:
                words = line.strip().split()
                ids = [lex[w] for w in words if w in lex]
                arr_ids = []
                for id, freq in sorted(Counter(ids).items()):
                    arr_ids.append('{}:{}'.format(id, freq))
                fout.write(' '.join(arr_ids)+'\n')
        

if __name__ == '__main__':
    import argparse

    desc = """
Script to make a training dataset (.dat/lex) from raw texts (space-separated word sequences)

examples:
python utils/%(prog)s -i data/test.txt -o out/test -m 1"""
    arg_parser = argparse.ArgumentParser(description=desc, formatter_class=argparse.RawDescriptionHelpFormatter)
    arg_parser.add_argument('-i', '--input', metavar='FILE',
                            help='input file (.txt) for raw texts')
    arg_parser.add_argument('-o', '--out_base', metavar='PREF',
                            help='output prefix of path to save .dat/lex files')
    arg_parser.add_argument('-m', '--min_freq', metavar='N', type=int, default=5,
                            help='minimum frequency for .lex file')

    args = arg_parser.parse_args()
    if not (args.input and args.out_base):
        arg_parser.print_help()
        exit()

    md = MakeData(raw_file=args.input, out_base=args.out_base, min_freq=args.min_freq)
    md.run()
