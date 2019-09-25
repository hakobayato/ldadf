import unittest

import sys
from os.path import dirname, abspath
sys.path.insert(0, dirname(abspath(__file__)) + '/../')

from compiler import LinkCompiler

class TestLinkCompiler(unittest.TestCase):
    def setUp(self):
        self.dnf = LinkCompiler(debug=False)
        self.lexer = self.dnf.lexer
        self.parser = self.dnf.parser

    def test_link(self):
        dnf = self.parser.parse('ML(a,b)')
        ref = 'Ep(a,b)'
        self.assertEqual(ref, str(dnf))
        dnf = self.parser.parse('CL(a,b)')
        ref = 'Np(a) | Np(b)'
        self.assertEqual(ref, str(dnf))

    def test_link2(self):
        dnf = self.parser.parse('IL(a,b)')
        ref = 'Ep(a,b) | Np(a)'
        self.assertEqual(ref, dnf)
        dnf = self.parser.parse('ISL(a,b,c)')
        ref = 'Np(a)&Np(b)&Np(c)'
        self.assertEqual(ref, dnf)
        dnf = self.parser.parse('XML(a,b,c)')
        ref = 'Ep(a,b,c)'
        self.assertEqual(ref, dnf)
        dnf = self.parser.parse('XCL(a,b,c)')
        ref = 'Np(a) | Np(b) | Np(c)'
        self.assertEqual(ref, dnf)
        dnf = self.parser.parse('XIL(a,b,c)')
        ref = 'Ep(a,b,c) | Np(a) | Np(b)'
        self.assertEqual(ref, dnf)

    def test_parse(self):
        dnf = self.parser.parse('ML(a,b) & (CL(a,c) | CL(b,c))')
        ref = 'Ep(a,b)&Np(c) | Ep(a,b)&Np(c) | Np(a)&Np(b) | Np(a)&Np(b)'
        self.assertEqual(ref, str(dnf))

    def test_parse2(self):
        dnf = self.parser.parse('XML(a,b,c) & XML(d,e,f) & XML(i,j,k) & XCL(a,d,i) & ISL(x,y,z)')
        ref = ' | '.join(['Ep(a,b,c)&Ep(d,e,f)&Np(i)&Np(j)&Np(k)&Np(x)&Np(y)&Np(z)',
                          'Ep(a,b,c)&Ep(i,j,k)&Np(d)&Np(e)&Np(f)&Np(x)&Np(y)&Np(z)',
                          'Ep(d,e,f)&Ep(i,j,k)&Np(a)&Np(b)&Np(c)&Np(x)&Np(y)&Np(z)'])
        self.assertEqual(ref, str(dnf))

    def rand_links(self, n_words, n_links, n_loops, seed=0):
        assert n_words < 50 and n_links < 20 and n_loops < 500, 'too big number!'
        import random
        random.seed(seed)
        words = list(range(n_words))
        for n in range(n_loops):
            links = []
            for l in range(n_links):
                word1, word2 = random.sample(words, 2)
                link = random.choice(['ML', 'CL'])
                links.append('{}({},{})'.format(link, word1, word2))
            yield('&'.join(links))

    def test_rand_links(self):
        for link in self.rand_links(10, 5, 100, 0):
            dnf = self.parser.parse(link)
            ref = dnf.copy()
            dnf.shrink()
            words = dnf.words()
            self.assertEqual(ref.atf(words), dnf.atf(words))

    def all_links(self, n_words, n_links):
        assert n_words < 50 and n_links < 20, 'too big number!'
        from itertools import combinations
        words = list(range(n_words))
        links = []
        for word1, word2 in combinations(words, 2):
            for link in ['ML', 'CL']:
                links.append('{}({},{})'.format(link, word1, word2))
        for sublinks in combinations(links, n_links):
            yield('&'.join(sublinks))

    def proc_all_links(self, n_words, n_links):
        for link in self.all_links(n_words, n_links):
            dnf = self.parser.parse(link)
            ref = dnf.copy()
            dnf.shrink()
            words = dnf.words()
            self.assertEqual(ref.atf(words), dnf.atf(words))

    def test_all_links(self):
        for n_words in range(2, 5):
            for n_links in range(1, n_words * 2+1):
                if n_links > 5: continue
                # print('checking all combinations for (n_words={}, n_links={})..'.format(n_words, n_links))
                self.proc_all_links(n_words, n_links)

    def worst_links(self, n_words, n_links, n_loops, seed=0):
        assert n_words < 50 and n_links < 20 and n_loops < 500, 'too big number!'
        import random
        random.seed(seed)
        words = list(range(n_words))
        for n in range(n_loops):
            links = []
            for l in range(n_links):
                word1, word2 = random.sample(words, 2)
                links.append('{}({},{})'.format('CL', word1, word2))
            yield('&'.join(links))

    def test_worst_links(self):
        for link in self.worst_links(10, 5, 100, 0):
            dnf = self.parser.parse(link)
            ref = dnf.copy()
            dnf.shrink()
            words = dnf.words()
            self.assertEqual(ref.atf(words), dnf.atf(words))

    def _test_comp_shrink(self):
        # #conj of before/after shrinking
        # before: 51.7200
        # after:  2.6100
        n_words = 10
        n_links = 10
        n_loops = 100
        seed = 0

        num_dnf = 0
        num_ref = 0
        for link in self.rand_links(n_words, n_links, n_loops, seed):
            dnf = self.parser.parse(link)
            ref = dnf.copy()
            dnf.shrink()
            num_dnf += len(dnf.dnf)
            num_ref += len(ref.dnf)
        print('#conj of before/after shrinking')
        print('before: {:.4f}'.format(num_ref/n_loops))
        print('after:  {:.4f}'.format(num_dnf/n_loops))

    def _test_online_shrink(self):
        #              sum    (=parse+shrink)
        # time batch:  0.4949 (=0.4366+0.0583)
        # time online: 0.2988 (=0.2965+0.0022)
        n_words = 10
        n_links = 10
        n_loops = 400
        seed = 0

        lc_batch = LinkCompiler(debug=False, shrink=False)
        lc_online = LinkCompiler(debug=False, shrink=True)

        import time
        time_parse_batch = 0
        time_shrink_batch = 0
        time_parse_online = 0
        time_shrink_online = 0
        for link in self.rand_links(n_words, n_links, n_loops, seed):
            start = time.time()
            dnf = lc_batch.parser.parse(link)
            time_parse_batch += time.time()-start
            start = time.time()
            dnf.shrink()
            time_shrink_batch += time.time()-start

            start = time.time()
            dnf = lc_online.parser.parse(link)
            time_parse_online += time.time()-start
            start = time.time()
            dnf.shrink()
            time_shrink_online += time.time()-start

        print('             sum    (=parse+shrink)')
        print('time batch:  {:.4f} (={:.4f}+{:.4f})'.format(time_parse_batch+time_shrink_batch, time_parse_batch, time_shrink_batch))
        print('time online: {:.4f} (={:.4f}+{:.4f})'.format(time_parse_online+time_shrink_online, time_parse_online, time_shrink_online))


if __name__ == '__main__':
    unittest.main()
