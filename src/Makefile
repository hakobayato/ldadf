CC	= g++
CFLAGS	= -O0 -g3
CFLAGSR	= -O2 -s -DNDEBUG
LDFLAGS	= -lm

SRCS	= utils.cc lda.cc dtree.cc ldadf.cc
OBJS	= $(SRCS:.cc=.o)

TESTGEN = cxxtestgen
TESTS	= $(wildcard tests/test_*.h)

all: ldadf

ldadf: main.cc $(OBJS) depend
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(OBJS)

.cc.o:
	$(CC) $(CFLAGS) -c $<

test: test.cc $(OBJS) depend
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(OBJS)

test.cc: $(TESTS)
	$(TESTGEN) --error-printer -o $@ $(TESTS)

depend:
	$(CC) -MM $(SRCS) > depend

clean:
	rm -f ldadf test
	rm -f test.cc test.tmp
	rm -f depend
	rm -f *~ *.o \#*\#

release: CFLAGS=$(CFLAGSR)
release: clean all

-include depend
