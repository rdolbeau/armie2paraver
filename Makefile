SRCC=armietraceparser.c armstuff.c
SRCPP=armstuff_llvm.cc
OBJS=$(SRCC:%.c=%.o) $(SRCPP:%.cc=%.o) armietraceparser_lex.o armietraceparser_yacc.o

LLVM=/opt/local/llvm-10.0.0

CC=gcc
CFLAGS=-O1 -g
CXX=g++
CXXFLAGS=-O1 -g $(shell ${LLVM}/bin/llvm-config  --cxxflags)
CXXLIBS=$(shell ${LLVM}/bin/llvm-config  --ldflags --system-libs --libs)
LEX=flex
YACC=bison -d #--report-file=bison.log --report=all

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

armietraceparser: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@  $(CXXLIBS)

armietraceparser.o: armietraceparser_yacc.h

armstuff.o: armstuff.h

armietraceparser_yacc.h: armietraceparser_yacc.o

armietraceparser_yacc.o: armietraceparser_yacc.y armstuff.h
	$(YACC) -o $(<:%.y=%.c) $<
	$(CC) $(CFLAGS) $(<:%.y=%.c) -c -o $@

armietraceparser_lex.o: armietraceparser_lex.l armietraceparser_yacc.h armstuff.h
	$(LEX) -o $(<:%.l=%.c) $<
	$(CC) $(CFLAGS) $(<:%.l=%.c) -c -o $@

clean:
	rm -f $(OBJS) armietraceparser armietraceparser_lex.c armietraceparser_yacc.c armietraceparser_yacc.h
