import unittest

import sys
from os.path import dirname, abspath
sys.path.insert(0, dirname(abspath(__file__)) + '/../')

from parser import LinkParser

class TestLinkParser(unittest.TestCase):
    def setUp(self):
        self.lp = LinkParser(debug=False)
        self.lexer = self.lp.lexer
        self.parser = self.lp.parser
        self.data_dir = dirname(__file__)+'/../../data'

    def test_token(self):
        self.lexer.input('ML(a,b) | CL ( c, d )')
        values = ('ML', '(', 'a', ',', 'b', ')', '|', 'CL', '(', 'c', ',', 'd', ')')
        types = ('LINK', '(', 'WORD', ',', 'WORD', ')', '|', 'LINK', '(', 'WORD', ',', 'WORD', ')')
        for i, token in enumerate(self.lexer):
            self.assertEqual(values[i], token.value)
            self.assertEqual(types[i], token.type)

    def test_str(self):
        self.lexer.input('ML("CL\'s","()")')
        values = ('ML', '(', 'CL\'s', ',', '()', ')')
        types = ('LINK', '(', 'STR', ',', 'STR', ')')
        for i, token in enumerate(self.lexer):
            self.assertEqual(values[i], token.value)
            self.assertEqual(types[i], token.type)

    def test_link(self):
        dnf = self.parser.parse('ML(a,b)')
        ref = [['Ep(a,b)']]
        self.assertEqual(ref, dnf)
        dnf = self.parser.parse('CL(a,b)')
        ref = [['Np(a)'], ['Np(b)']]
        self.assertEqual(ref, dnf)

    def test_link2(self):
        dnf = self.parser.parse('IL(a,b)')
        ref = [['Ep(a,b)'], ['Np(a)']]
        self.assertEqual(ref, dnf)
        dnf = self.parser.parse('ISL(a,b,c)')
        ref = [['Np(a)', 'Np(b)', 'Np(c)']]
        self.assertEqual(ref, dnf)
        dnf = self.parser.parse('XML(a,b,c)')
        ref = [['Ep(a,b,c)']]
        self.assertEqual(ref, dnf)
        dnf = self.parser.parse('XCL(a,b,c)')
        ref = [['Np(a)'], ['Np(b)'], ['Np(c)']]
        self.assertEqual(ref, dnf)
        dnf = self.parser.parse('XIL(a,b,c)')
        ref = [['Ep(a,b,c)'],['Np(a)'],['Np(b)']]
        self.assertEqual(ref, dnf)

    def test_parse(self):
        dnf = self.parser.parse('ML(a,b) & (CL(a,c) | CL(b,c))')
        ref = [['Ep(a,b)', 'Np(a)'], ['Ep(a,b)', 'Np(c)'], ['Ep(a,b)', 'Np(b)'], ['Ep(a,b)', 'Np(c)']]
        self.assertEqual(ref, dnf)

    def test_load_lex(self):
        lex = self.lp.load_lex(self.data_dir+'/test.lex')
        ref = {'A':0, 'B':1, 'C':2}
        self.assertEqual(ref, lex)

    def test_parse_lex(self):
        self.lp = LinkParser(debug=False, lex_file=self.data_dir+'/test.lex')
        self.lexer = self.lp.lexer
        self.parser = self.lp.parser
        dnf = self.parser.parse('ML("A","B") & (CL("A","C") | CL("B","C"))')
        ref = [['Ep(0,1)', 'Np(0)'], ['Ep(0,1)', 'Np(2)'], ['Ep(0,1)', 'Np(1)'], ['Ep(0,1)', 'Np(2)']]
        self.assertEqual(ref, dnf)


if __name__ == '__main__':
    unittest.main()
