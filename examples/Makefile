#CXX=g++-4.8
#CXXFLAGS=-std=c++11 -g -O2
#CXXFLAGS=-std=c++11 -g -O2 -DNVARIADIC
#LDLIBS=-ltbb

CXX=icpc
CXXFLAGS=-std=c++11 -gxx-name=g++-4.6 -g -ip -ansi-alias -O2 -tbb
#CXXFLAGS=-std=c++11 -gxx-name=g++-4.6 -g -ip -ansi-alias -O2 -tbb -DNVARIADIC

.PHONY: all clean

all: test1 ant members inheritance ptest1 ittest1 multi

clean:
	rm test1 ant members inheritance ptest1 ittest1 multi
