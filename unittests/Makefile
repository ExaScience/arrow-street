#CXX=g++-4.8
#CXXFLAGS=-std=c++11 -g -O2 -ltbb
#CXXFLAGS=-std=c++11 -g -O2 -ltbb -DNVARIADIC
#LDLIBS=-ltbb

CXX=icpc
CXXFLAGS=-std=c++11 -gxx-name=g++-4.6 -g -ip -ansi-alias -O2 -tbb
#CXXFLAGS=-std=c++11 -gxx-name=g++-4.6 -g -ip -ansi-alias -O2 -tbb -DNVARIADIC

.PHONY: all clean

all: utests utestsn

clean:
	rm utests utestsn
