main:

%: src/*.cpp
	g++ -Wall -o $*.out src/*.cpp
