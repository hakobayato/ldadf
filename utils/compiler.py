import os

from dnf import DNF, Conj
from parser import LinkParser, LinkParserException

class LinkCompiler(LinkParser):
    def __init__(self, debug=False, lex_file=None, online_shrink=False, verbose=False):
        super().__init__(debug, lex_file)
        self.online_shrink = online_shrink
        self.verbose = verbose

    def gen_dnf(self, link, words):
        if link == 'ML': # ML(A,B) = Ep(A,B)
            return DNF([Conj(eps=[words])])
        elif link == 'CL': # CL(A,B) = Np(A) | Np(B)
            return DNF([Conj(np=[words[0]]), Conj(np=[words[1]])])
        elif link == 'IL': # IL(A,B) = Ep(A,B) | Np(A)
            return DNF([Conj(eps=[words]), Conj(np=[words[0]])])
        elif link == 'ISL': # ISL(A,B,..) = Np(A) & Np(B) & ...
            return DNF([Conj(np=words)])
        elif link == 'XML': # XML(A,B,..) = Ep(A,B,..)
            return DNF([Conj(eps=[words])])
        elif link == 'XCL': # XCL(A,B,C) = Np(A) | Np(B) | ...
            return DNF([Conj(np=[word]) for word in words]) # XCL = not XML
        elif link == 'XIL': # XIL(A,B,..,Y) = Np(A) | Np(B) | ... | Ep(A,B..,,Y)
            return DNF([Conj(eps=[words])] + [Conj(np=[word]) for word in words[:-1]])
        else:
            raise LinkParserException('Unknown link: {}'.format(link))

    def and_dnf(self, dnf1, dnf2):
        dnf = dnf1 & dnf2
        if self.online_shrink:
            dnf.shrink()
        return dnf

    def or_dnf(self, dnf1, dnf2):
        dnf = dnf1 | dnf2
        if self.online_shrink:
            dnf.shrink()
        return dnf

    def compile(self, text):
        dnf = self.parser.parse(text)
        dnf.shrink()
        if self.verbose:
            print('compiled DNF: {}'.format(dnf))
            for i, conj in enumerate(dnf):
                print('dtree-{}: {}'.format(i, conj))
        return dnf

    def compile_file(self, link_file, dnf_file):
        link_text = ''
        if os.path.exists(link_file):
            with open(link_file) as f:
                link_text = f.read()
        else:
            link_text = link_file # allow raw text of links for convenience
        dnf = self.compile(link_text)
        dnf.save(dnf_file)


def input_mode(compiler):
    while 1:
        try:
            text = input('Input links> ')
            if not text:
                print('Example: ML(a,b)&(CL(a,c)|CL(b,c))')
                continue
            dnf = compiler.compile(text)
            print('dnf primitives: {}'.format(dnf))
            print('dnf file:')
            for conj in dnf.dnf:
                print(conj.get_save_str())
        except EOFError:
            print()
            break
        except LinkParserException as e:
            print(e)
            continue


if __name__ == '__main__':
    import argparse
    desc = """
Compiler from constraint linkes to DNF of primitives

examples:
python utils/%(prog)s -c data/test.cst -d data/test.dnf -l data/test.lex -v
python utils/%(prog)s -I"""
    arg_parser = argparse.ArgumentParser(description=desc, formatter_class=argparse.RawDescriptionHelpFormatter)
    arg_parser.add_argument('-l', '--lex', metavar='FILE',
                            help='lexicon file (.lex) for .dat file')
    arg_parser.add_argument('-c', '--cst', metavar='FILE',
                            help='file (.cst) to load constraint links')
    arg_parser.add_argument('-d', '--dnf', metavar='FILE',
                            help='file (.dnf) to save compiled DNF')
    arg_parser.add_argument('-I', '--interactive', action='store_true',
                            help='interactive mode for debugging used without any other options')
    arg_parser.add_argument('--debug', action='store_true',
                            help='debug mode for ply parser')
    arg_parser.add_argument('-v', '--verbose', action='store_true',
                            help='verbose mode')
    args = arg_parser.parse_args()

    compiler = LinkCompiler(debug=args.debug, online_shrink=True, lex_file=args.lex, verbose=args.verbose)
    if args.interactive:
        input_mode(compiler)
    else:
        if not (args.cst and args.dnf):
            arg_parser.print_help()
            exit()
        compiler.compile_file(args.cst, args.dnf)
