from ply import lex, yacc

class LinkParserException(Exception):
    pass

class LinkParser:
    def __init__(self, debug=False, lex_file=None):
        self.debug = debug
        self.lexer = lex.lex(module=self, debug=self.debug)
        #self.parser = yacc.yacc(module=self, debug=self.debug, tabmodule='parser_parsetab') # generate tables only once
        self.parser = yacc.yacc(module=self, debug=self.debug, write_tables=False) # see parser.out when debugging

        self.lex_file = lex_file
        self.lex2id = None
        if lex_file:
            self.lex2id = self.load_lex(lex_file)

    def load_lex(self, lex_file):
        lex2id = {}
        with open(lex_file) as f:
            for i, line in enumerate(f):
                word = line.strip()
                lex2id[word] = i
        return lex2id

    # * Lex definition

    literals = '()&|,'
    tokens = ('WORD', 'STR', 'LINK')

    t_LINK = r'XML|XCL|XIL|ML|CL|IL|ISL'
    t_ignore = " \t\n"

    def t_WORD(self, t):
        r'[0-9a-z]+'
        if self.lex2id:
            if t.value not in self.lex2id:
                raise LinkParserException("Unknown word '{}', not in {}".format(t.value, self.lex_file))
            t.value = self.lex2id[t.value]
        return t

    def t_STR(self, t):
        r'"[^"]+"'
        t.value = t.value[1:-1] # strip quotes (")
        t = self.t_WORD(t)
        return t

    def t_error(self, t):
        raise LinkParserException("Illegal character '{}'".format(t.value[0]))

    # * Yacc definition
    
    precedence = ( # '&' is applied before '|'
        ('left', '|'),
        ('left', '&'),
    )

    def p_expression_and(self, p):
        "expression : expression '&' expression"
        p[0] = self.and_dnf(p[1], p[3])

    def p_expression_or(self, p):
        "expression : expression '|' expression"
        p[0] = self.or_dnf(p[1], p[3])

    def p_expression_group(self, p):
        "expression : '(' expression ')'"
        p[0] = p[2]

    def p_expression_link(self, p):
        "expression : LINK '(' word_list ')'"
        p[0] = self.gen_dnf(p[1], p[3])

    def p_word_list(self, p):
        "word_list : word_list ',' word_list \n| word "
        if len(p) == 2:
            p[0] = [p[1]]
        else:
            p[0] = p[1] + p[3]

    def p_word(self, p):
        "word : WORD \n| STR"
        p[0] = p[1]

    def p_error(self, p):
        at = 'EOF' if not p else p.value
        raise LinkParserException("Syntax error at '{}'".format(at))

    # * Link translation (to be overridden)

    def gen_dnf(self, link, words):
        if link == 'ML': # ML(A,B) = Ep(A,B)
            return [['Ep({},{})'.format(words[0], words[1])]]
        elif link == 'CL': # CL(A,B) = Np(A) | Np(B)
            return [['Np({})'.format(words[0])], ['Np({})'.format(words[1])]]
        elif link == 'IL': # IL(A,B) = Ep(A,B) | Np(A)
            return [['Ep({},{})'.format(words[0], words[1])], ['Np({})'.format(words[0])]]
        elif link == 'ISL': # ISL(A,B,..) = Np(A) & Np(B) & ...
            return [['Np({})'.format(word) for word in words]]
        elif link == 'XML': # XML(A,B,..) = Ep(A,B,..)
            return [['Ep({})'.format(','.join(words))]]
        elif link == 'XCL': # XCL(A,B,C) = Np(A) | Np(B) | ...
            return [['Np({})'.format(word)] for word in words] # XCL = not XML
        elif link == 'XIL': # XIL(A,B,..,Y) = Np(A) | Np(B) | ... | Ep(A,B..,,Y)
            return [['Ep({})'.format(','.join(words))]] + [['Np({})'.format(word)] for word in words[:-1]]
        else:
            raise LinkParserException('Unknown link: {}'.format(link))

    def and_dnf(self, dnf1, dnf2):
        new_dnf = []
        for c1 in dnf1:
            for c2 in dnf2:
                new_dnf.append(c1 + c2)
        return new_dnf

    def or_dnf(self, dnf1, dnf2):
        return dnf1 + dnf2
