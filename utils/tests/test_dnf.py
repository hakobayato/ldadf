import unittest

import sys
from os.path import dirname, abspath
sys.path.insert(0, dirname(abspath(__file__)) + '/../')

from dnf import DNF, Conj

class TestConj(unittest.TestCase):
    def test_add_ep(self):
        c = Conj()
        c.add_ep([1,2])
        c.add_ep([3,4])
        c.add_ep([1,3])
        self.assertEqual([{1,2,3,4}], c.eps)

    def test_add_np(self):
        c = Conj()
        c.add_np(1)
        c.add_np(2)
        c.add_np(1)
        self.assertEqual({1,2}, c.np)

    def test_add_ep_np(self):
        r = Conj([],[1,2])
        c = Conj([[1,2]])
        c.add_np(1)
        self.assertEqual(r, c)
        c = Conj([],[1])
        c.add_ep([1,2])
        self.assertEqual(r, c)

    def test_eq(self):
        c1 = Conj([[1,2]],[3])
        c2 = Conj([],[])
        c3 = Conj([[1,2,4]],[3,5])
        c4 = Conj([[1,2],[4,5]],[3])
        self.assertEqual(True, c1 == c1)
        self.assertEqual(False, c1 == c2)
        self.assertEqual(False, c1 == c3)
        self.assertEqual(False, c1 == c4)
        c1 = Conj([[1,2],[3,4]],[])
        c2 = Conj([[3,4],[1,2]],[])
        self.assertEqual(True, c1 == c2)

    def test_is_subconj(self):
        c1 = Conj([[1,2]],[3])
        c2 = Conj([[1,2,4]],[3,5])
        c3 = Conj([[1,2]],[])
        self.assertEqual(True, c1.is_subconj(c1))
        self.assertEqual(True, c1.is_subconj(c2))
        self.assertEqual(False, c1.is_subconj(c3))

    def test_is_subconj_ep(self):
        c1 = Conj([[1,2]],[3])
        c2 = Conj([[1,2,4]],[3,5])
        c3 = Conj([[1,2]],[])
        c4 = Conj([],[1,2,3])
        self.assertEqual(True, c1.is_subconj_ep(c1))
        self.assertEqual(True, c1.is_subconj_ep(c2))
        self.assertEqual(False, c1.is_subconj_ep(c3))
        self.assertEqual(True, c1.is_subconj_ep(c4))
        c1 = Conj([[1,2,3],[4,5,6]],[7,8])
        c2 = Conj([[1,2,3]],[4,5,6,7,8])
        c3 = Conj([[1,2,3],[4,5]],[6,7,8])
        c4 = Conj([[1,2]],[4,5,6,7,8])
        c5 = Conj([[1,2,3]],[4,5,6,7])
        self.assertEqual(True, c1.is_subconj_ep(c1))
        self.assertEqual(True, c1.is_subconj_ep(c2))
        self.assertEqual(False, c1.is_subconj_ep(c3))
        self.assertEqual(False, c1.is_subconj_ep(c4))
        self.assertEqual(False, c1.is_subconj_ep(c5))

    def test_repr(self):
        c1 = Conj([[1,2]],[3])
        self.assertEqual('Ep(1,2)&Np(3)', str(c1))

    def test_and(self):
        c1 = Conj([[1,2]],[3,4])
        c2 = Conj([[1,2,3], [4,5]],[3])
        c3 = Conj([[1,2,3], [4,5]],[3,4])
        self.assertEqual(c1, c1 & c1)
        self.assertEqual(c3, c1 & c2)
        c1 = Conj([],[1])
        c2 = Conj([],[2])
        c3 = Conj([],[1,2])
        self.assertEqual(c3, c1 & c2)
        c1 = Conj([],[1,2])
        c2 = Conj([[1,2]],[])
        c3 = Conj([],[1,2])
        self.assertEqual(c3, c1 & c2)
        self.assertEqual(c3, c2 & c1)
        c1 = Conj([[1,2]],[])
        c2 = Conj([],[3])
        c3 = Conj([[1,2]],[3])
        self.assertEqual(c3, c1 & c2)
        self.assertEqual(c3, c2 & c1)
        c1 = Conj([[3,4]], [1])
        c2 = Conj([[1,4]], [])
        c3 = Conj([], [1,3,4])
        self.assertEqual(c3, c1 & c2)
        self.assertEqual(c3, c2 & c1)

    def test_powerset(self):
        c = Conj()
        p = c.powerset([1,2])
        self.assertEqual([[],[1],[2],[1,2]], list(p))

    def test_words(self):
        c = Conj([[1,2]], [3])
        self.assertEqual([1,2,3], c.words())

    def test_atf(self):
        c = Conj([[1,2]], [3])
        ref = [[], [1,2]]
        ref = set(map(frozenset, ref))
        self.assertEqual(ref, c.atf([1,2,3]))
        # Fact (1): X | (X & Y) = X
        c1 = Conj([], [1])
        c2 = Conj([], [1,2])
        self.assertEqual(c1.atf([1,2]), c1.atf([1,2]) | c2.atf([1,2]))
        # Proposition (a): Ep(A,B) | (Np(A) & Np(B)) = Ep(A,B)
        c1 = Conj([[1,2]], [])
        c2 = Conj([], [1,2])
        self.assertEqual(c1.atf([1,2]), c1.atf([1,2]) | c2.atf([1,2]))

    def test_get_save_str(self):
        c = Conj([[1,2],[3,4]], [5,6])
        ref = '1,2:3,4;5,6'
        self.assertEqual(ref, c.get_save_str())

class TestDNF(unittest.TestCase):
    def setUp(self):
        pass

    def test_init(self):
        dnf = DNF([Conj([[1,2]],[]), Conj([],[3])])
        ref = ['Ep(1,2)', 'Np(3)']
        self.assertEqual(ref, list(map(str, dnf.dnf)))

    def test_repr(self):
        dnf = DNF([Conj([[1,2]],[]), Conj([],[3])])
        ref = 'Ep(1,2) | Np(3)'
        self.assertEqual(ref, str(dnf))

    def test_eq(self):
        dnf1 = DNF([Conj([[1,2]],[]), Conj([],[3])])
        dnf2 = DNF([Conj([],[3]), Conj([[1,2]],[])])
        self.assertEqual(dnf1, dnf1)
        self.assertEqual(dnf1, dnf2)

    def test_and(self):
        dnf1 = DNF([Conj([[1,2]],[])])
        dnf2 = DNF([Conj([],[2]), Conj([],[3])])
        ref = DNF([Conj([[1,2]],[3]), Conj([],[1,2])])
        self.assertEqual(ref, dnf1 & dnf2)
        dnf1 = DNF([Conj([[1,2,3],[4,5,6]],[])])
        dnf2 = DNF([Conj([[7,8,9]],[])])
        ref = DNF([Conj([[1,2,3],[4,5,6],[7,8,9]],[])])
        self.assertEqual(ref, dnf1 & dnf2)

    def test_or(self):
        dnf1 = DNF([Conj([[1,2]],[])])
        dnf2 = DNF([Conj([],[2]), Conj([],[3])])
        ref = DNF([Conj([[1,2]],[]), Conj([],[2]), Conj([],[3])])
        self.assertEqual(ref, dnf1 | dnf2)

    def test_atf(self):
        dnf = DNF([Conj([[1,2]],[]), Conj([],[1,2])])
        ref = [[], [1,2]]
        ref = set(map(frozenset, ref))
        words = [1,2]
        self.assertEqual(ref, dnf.atf(words))
        dnf = DNF([Conj([[1,2]],[3]), Conj([],[0,1,2])])
        ref = [[], [0], [3], [1,2], [0,1,2]]
        ref = set(map(frozenset, ref))
        words = [0,1,2,3]
        self.assertEqual(ref, dnf.atf(words))

    def test_shrink(self):
        dnf = DNF([Conj([], [1,2]), Conj([[1,2]], [])])
        ref = DNF([Conj([[1,2]], [])])
        dnf.shrink()
        self.assertEqual(ref, dnf)
        dnf = DNF([Conj([[1,2],[3,4]], [5]), Conj([[3,4,5]], [1,2,5,6])])
        ref = DNF([Conj([[1,2],[3,4]], [5])])
        dnf.shrink()
        self.assertEqual(ref, dnf)
        dnf = DNF([Conj([[3,4]], [2]), Conj([], [3,4])])
        ref = dnf.copy()
        dnf.shrink()
        self.assertEqual(ref, dnf)

    def test_shrink_atf(self):
        dnf = DNF([Conj([[1,2],[3,4]], [5]), Conj([[3,4,5]], [1,2,5,6])])
        ref = DNF([Conj([[1,2],[3,4]], [5])])
        words = [1,2,3,4,5,6]
        self.assertEqual(ref.atf(words), dnf.atf(words))
        dnf = DNF([Conj([[4,1],[2,1],[4,2]], [])])
        ref = dnf.copy()
        dnf.shrink()
        words = dnf.words()
        self.assertEqual(ref.atf(words), dnf.atf(words))

    def test_words(self):
        dnf = DNF([Conj([[1,2]], [3,4]), Conj([[1,2],[4,5]], [6])])
        ref = [1,2,3,4,5,6]
        self.assertEqual(ref, dnf.words())

    def test_save(self):
        dnf = DNF([Conj([[1,2],[3,4]], [5]), Conj([[6,7]], []), Conj([], [8,9])])
        ref = '1,2:3,4;5\n6,7;\n;8,9\n'
        tmpfile = 'tmp.df'
        dnf.save(tmpfile)
        with open(tmpfile) as f:
            self.assertEqual(ref, f.read())
        import os
        os.remove(tmpfile)

    def test_load(self):
        tmpfile = 'tmp.df'
        with open(tmpfile, 'w') as f:
            f.write('1,2:3,4;5\n6,7;\n;8,9')
        dnf = DNF()
        dnf.load(tmpfile)
        ref = DNF([Conj([[1,2],[3,4]], [5]), Conj([[6,7]], []), Conj([], [8,9])])
        self.assertEqual(ref, dnf)
        import os
        os.remove(tmpfile)


if __name__ == '__main__':
    unittest.main()
