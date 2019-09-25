class DNF:
    def __init__(self, dnf=None):
        if not dnf: dnf = []
        self.dnf = dnf

    def shrink(self):
        # reduce the number of conjunctions by the following rules:
        # Fact (1): X | (X & Y) = X
        # Proposition (a): Ep(A,B) | (Np(A) & Np(B)) = Ep(A,B)
        for i in range(len(self.dnf)-1):
            if not self.dnf[i]: # already removed
                continue
            for j in range(i+1, len(self.dnf)):
                if not self.dnf[j]: # already removed
                    continue
                if self.dnf[j].is_subconj_ep(self.dnf[i]):
                    self.dnf[i] = None
                    break # move to dnf[i+1]
                if self.dnf[i].is_subconj_ep(self.dnf[j]):
                    self.dnf[j] = None
        self.dnf = list(filter(None, self.dnf))

    def save(self, filename):
        with open(filename, 'w') as f:
            for conj in self.dnf:
                f.write(conj.get_save_str()+'\n')

    def load(self, filename, id2word=None):
        dnf = []
        with open(filename) as f:
            for line in f:
                eps, np = line.strip().split(';')
                eps = [ep.split(',') for ep in eps.split(':')] if eps != '' else []
                np = np.split(',') if np != '' else []
                if id2word: # for viewer
                    for i, ep in enumerate(eps): eps[i] = self.ids_to_words(ep, id2word)
                    np = self.ids_to_words(np, id2word)
                dnf.append(Conj(eps, np))
        self.dnf = dnf

    def ids_to_words(self, ids, id2word):
        words = []
        for wid in ids:
            wid = int(wid)
            if wid in id2word:
                words.append(id2word[wid])
            else:
                exit('Unknown word id: {}'.format(wid))
        return words

    def words(self):
        words = set()
        for conj in self.dnf:
            words |= set(conj.words())
        return sorted(words)

    def atf(self, words): # asymptotic topic family on given words
        atf = set()
        for conj in self.dnf:
            atf |= conj.atf(words)
        return atf

    def copy(self):
        return DNF([conj.copy() for conj in self.dnf])

    def __and__(self, other):
        new_dnf = DNF()
        for c1 in self.dnf:
            for c2 in other.dnf:
                new_dnf.dnf.append(c1 & c2)
        return new_dnf

    def __or__(self, other):
        new_dnf = self.copy()
        new_dnf.dnf += other.copy().dnf
        return new_dnf

    def __eq__(self, other):
        return str(self) == str(other)

    def __repr__(self):
        conj_list = map(str, self.dnf)
        return ' | '.join(sorted(conj_list))

    def __iter__(self):
        for conj in self.dnf:
            yield conj

    def __getitem__(self, key):
        return self.dnf[key]

    def __len__(self):
        return len(self.dnf)


class Conj:
    def __init__(self, eps=None, np=None):
        self.eps = []
        self.np = set()
        if eps:
            for words in eps:
                self.add_ep(words)
        if np:
            for word in np:
                self.add_np(word)

    def add_ep(self, words):
        assert isinstance(words, (list, tuple, set)), 'Ep needs a list/tuple/set'
        assert len(words) >= 2, 'Ep needs at least two words'
        new_ep = set(words)
        for i, ep in enumerate(self.eps):
            if new_ep & ep:
                # Fact (0): Ep(A,B) & Ep(B,C) = Ep(A,B,C)
                new_ep |= ep
                self.eps[i] = None
        self.eps = list(filter(None, self.eps))

        if new_ep & self.np:
            # Proposision (b): Ep(A,B) & Np(A) = Np(A) & Np(B)
            self.np |= new_ep
        else:
            self.eps.append(new_ep)

    def add_np(self, word):
        for i, ep in enumerate(self.eps):
            if word in ep:
                # Proposision (b): Ep(A,B) & Np(A) = Np(A) & Np(B)
                self.np |= ep
                self.eps.pop(i)
                break
        else:
            self.np.add(word)

    def is_subconj(self, other): # obsolete
        # Fact (1): (X & Y) | X = X
        if not self.np <= other.np:
            return False
        for ep in self.eps:
            for ep2 in other.eps:
                if ep <= ep2:
                    break
            else: # if not ep <= ep2
                return False
        return True

    def is_subconj_ep(self, other):
        # Fact (1): X | (X & Y) = X
        # Proposision (a): Ep(A,B) | (Np(A) & Np(B)) = Ep(A,B)

        # General form of Proposition (a) via Fact (1)
        # (P & Ep(A,B,..)) | (P & Np(A) & Np(B) & .. & Q)
        # = P & ( Ep(A,B,..) | (Np(A) & Np(B) & .. & Q) )
        # = P & ( (Ep(A,B,..) | (Np(A) & Np(B) & ..)) & (Ep(A,B,..) | Q) )
        # = P & ( Ep(A,B,..) & (Ep(A,B,..) | Q) )
        # = P & ( Ep(A,B,..) | (Ep(A,B,..) & Q) )
        # = P & Ep(A,B,..)

        if not self.np <= other.np: # if np is not in P
            return False

        np_dif = other.np - self.np # = Np(A) & Np(B) & .. & Q
        for ep in self.eps:
            for ep2 in other.eps:
                if ep <= ep2: # if ep is in P
                    break
            else: # if ep is not in P
                if not ep <= np_dif: # if ep is not Ep(A,B,..)
                    return False
        return True

    def get_save_str(self):
        save_str = ''
        eps_list = []
        for ep in self.eps:
            eps_list.append(','.join(map(str, ep)))
        save_str += ':'.join(eps_list)
        save_str += ';'
        save_str += ','.join(map(str, self.np))
        return save_str

    def words_to_ids(self, words, w2i): # obsolete
        if not lex:
            return words
        ids = []
        for word in words:
            assert word in w2i, 'Unkown word {}'.format(word)
            ids.append(w2i[word])
        return ids

    def words(self):
        words = []
        for ep in self.eps:
            words += list(ep)
        words += list(self.np)
        return sorted(words)

    def atf(self, words): # asymptotic topic family on words
        if self.eps == [] and self.np == []:
            return self.atf_minus(words, set())

        atf = self.atf_minus(words, self.np)
        for ep in self.eps:
            ep_atf = self.atf_minus(words, ep)
            for elem in ep_atf.copy():
                ep_atf.add(frozenset(elem | ep))
            atf &= ep_atf
        return atf

    def atf_minus(self, words, minus): # atf on words - minus
        atf = list(self.powerset(set(words) - set(minus)))
        return set(map(frozenset, atf)) # set of frozenset

    def powerset(self, words):
        if len(words) <= 0:
            yield []
        else:
            words = list(words)
            for subset in self.powerset(words[1:]):
                yield subset
                yield [words[0]] + subset

    def _powerset(self, words):
        from itertools import chain, combinations
        return chain.from_iterable(combinations(words, num) for num in range(len(words)+1))

    def copy(self):
        new_conj = Conj()
        new_conj.eps = [ep.copy() for ep in self.eps]
        new_conj.np = self.np.copy()
        return new_conj

    def __and__(self, other):
        new_conj = self.copy()
        for ep in other.eps:
            new_conj.add_ep(ep)
        for word in other.np:
            new_conj.add_np(word)
        return new_conj

    def __eq__(self, other):
        return str(self) == str(other)

    def __repr__(self):
        prims = []
        for ep in self.eps:
            ep_list = sorted(map(str, ep))
            prims.append('Ep({})'.format(','.join(ep_list)))
        for word in self.np:
            prims.append('Np({})'.format(word))
        return '&'.join(sorted(prims))
