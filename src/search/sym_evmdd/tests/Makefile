CXX = g++ -g -O3 -Wall -std=c++11 -DNDEBUG
TEST_BINARIES = $(basename $(wildcard *Test.cc))
HEADER = $(wildcard *.h)
OBJECTS = $(addsuffix .o, $(basename $(filter-out %Test.cc, $(wildcard *.cc))))
CPPLINT_PATH = ./cpplint.py

FILES = ../meddly_extensions/genericBinaryOperator.h \
	../meddly_extensions/intersectionMax.h \
	../meddly_extensions/unionMin.h \
	../meddly_extensions/userDefiniedOperators.h \
	../meddly_extensions/restrict.h \
	../meddly_extensions/swapVarPrimed.h \
	../meddly_extensions/lessThan.h \
	../meddly_extensions/lessThanApprox.h \
	../meddly_extensions/swapToPrimedVar.h \
	../meddly_extensions/partialComplement.h \
	../meddly_extensions/minVar.h \
        ../meddly_extensions/minStates.h \

.PRECIOUS: %.o

all: copyfiles compile test

compile: $(basename $(wildcard *Test.cc))

copyfiles:
	cp $(FILES) .
	cp $(FILES:.h=.cc) .

removecopiedfiles:
	rm -f $(notdir $(FILES))
	rm -f $(notdir $(FILES:.h=.cc))

test: $(basename $(wildcard *Test.cc)) 
	for T in $(basename $(wildcard *Test.cc)); do ./$$T; done

clean: removecopiedfiles
	rm -f *.o
	rm -f $(basename $(wildcard *Test.cc)) 
	rm -f *.class
	rm -f *Test.TMP.*
	rm -f core

%Test: %Test.o $(addsuffix .o, $(basename $(filter-out %Test.cc, $(wildcard *.cc))))
	$(CXX) -o $@ $^ -lgtest -lgtest_main -lpthread -lmeddly

%.o: %.cc $(wildcard *.h)
	$(CXX) -c $<

