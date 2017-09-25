CXX = g++
#CXXFLAGS = -O3 -march=native
CXXFLAGS = -std=c++11 -g
LIBRARIES = -lpthread
.PHONY: default run
default: run

run:
	${CXX} ${CXXFLAGS} *.cpp ${LIBRARIES} -o program

clean:
	rm -f *.o program
