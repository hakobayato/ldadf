import argparse
import glob
import os

cst_texts = [
    '',
    'ML("A","B")',
    'CL("B", "C")',
    'IL("B", "A")',
    'ML("A", "B") | ML("A", "C")',
    'IL("B", "A") & IL("C", "A")',
    '( ML("A", "B") | ML("A", "C") ) & CL("B", "C")',
    'IL("B", "A") & IL("C", "A") & CL("B", "C")',
    'ML("A", "B") & ML("A", "C")',
    'ML("A", "B") & CL("B", "C")',
]

def run_example(index, eta, verbose, seed):
    dat_file = './data/test.dat'
    lex_file = './data/test.lex'
    cst_text = ''

    if index >= 0 and index < len(cst_texts):
        cst_text = cst_texts[index]
    else:
        raise ValueError('Unknown index: {}'.format(index))

    run_ldadf(dat_file, lex_file, cst_text, eta, verbose, seed)

def run_ldadf(dat_file, lex_file, cst_text, eta, verbose, seed):
    print('= Parameters')
    print('dat_file: {}'.format(dat_file))
    print('lex_file: {}'.format(lex_file))
    print('cst_text: {}'.format(cst_text))
    print('eta: {}'.format(eta))
    print('seed: {}'.format(seed))
    print('verbose: {}'.format(verbose))
    print()

    os.makedirs('./out/example', exist_ok=True)
    out_pref = './out/example/test'
    for path in glob.glob(out_pref + '.*'):
        os.remove(path)

    dnf_file = out_pref + '.dnf'
    print('= Compile constraint links')
    if cst_text != '':
        cmd = 'python utils/compiler.py -c\'{}\' -l{} -d{}'.format(cst_text, lex_file, dnf_file)
        if verbose:
            cmd += ' -v'
        print(cmd)
        os.system(cmd)
    print()

    print('= Train LDADF with constraint links')
    topic, step, alpha, beta, loop, burn = 2, 1000, 1, 0.01, 10, 5
    if not os.path.exists('./src/ldadf'):
        os.system('cd src; make release')
    cmd = './src/ldadf -n{} -m{} -a{} -b{} -e{} -l{} -u{} -o{}'.format(topic, step, alpha, beta, eta, loop, burn, out_pref)
    if verbose:
        cmd += ' -v'
    if seed:
        cmd += ' -s{}'.format(seed)
    if cst_text != '':
        cmd += ' -d{}'.format(dnf_file)
    cmd += ' {}'.format(dat_file)
    print(cmd)
    os.system(cmd)
    print()

    print('= Learned parameters')
    cmd = 'python utils/viewer.py -p{}.final -l{}'.format(out_pref, lex_file)
    if cst_text != '':
        cmd += ' -d{}'.format(dnf_file)
    if verbose:
        cmd += ' -v'
    print(cmd)
    os.system(cmd)

def interactive_mode(desc, args):
    while 1:
        print('\nSelect a constraint:')
        print(desc)
        text = input('Index> ')
        try:
            index = int(text)
            run_example(index, args.eta, args.verbose, args.seed)
        except Exception as e:
            print(e)
            continue

def main():
    desc = """
Demonstration of topic modeling with logical constraints

examples:
python %(prog)s -I
python %(prog)s -i 6 -e 100 -s 0 -v

prepared constraint links:
"""
    cst_desc = ''
    for i, cst in enumerate(cst_texts):
        cst_desc += '{}) {}\n'.format(i, cst)

    arg_parser = argparse.ArgumentParser(description=desc+cst_desc,
                                         formatter_class=argparse.RawDescriptionHelpFormatter)
    arg_parser.add_argument('-i', '--index', metavar='N', default=-1, type=int,
                            help='index of prepared constraint links')
    arg_parser.add_argument('-e', '--eta', metavar='N', default=100, type=float,
                            help='eta parameter for strength of links')
    arg_parser.add_argument('-s', '--seed', metavar='N', type=int,
                            help='eta parameter for strength of links')
    arg_parser.add_argument('-v', '--verbose', action='store_true',
                            help='verbose mode')
    arg_parser.add_argument('-I', '--interactive', action='store_true',
                            help='interactive mode')
    args = arg_parser.parse_args()

    if args.interactive:
        interactive_mode(cst_desc, args)
    elif args.index >= 0:
        run_example(args.index, args.eta, args.verbose, args.seed)
    else:
        arg_parser.print_help()
        

if __name__ == '__main__':
    main()
