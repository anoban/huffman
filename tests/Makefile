CXX = /usr/bin/g++

CXXFLAGS = -Wall -Wextra -std=c++20 -march=tigerlake -mavx512f -ffast-math -mprefer-vector-width=512

INCLUDE_PATHS = -I./ -I./../include -I./googletest -I./googletest/include

DEBUG = -DDEBUG -D_DEBUG -O0 -g3

NODEBUG = -D_NDEBUG -DNDEBUG -O3 -g0

SOURCES = ./*.cpp ./googletest/src/gtest-all.cc

build:
	$(CXX) $(SOURCES) $(INCLUDE_PATHS) $(CXXFLAGS) $(NODEBUG) -o test.out

clean:
	rm *.o -f
	rm *.out -f
