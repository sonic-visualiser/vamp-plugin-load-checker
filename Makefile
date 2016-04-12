
all:	helper plugincandidates

CXXFLAGS	:= -Wall -Werror 

helper:	helper.o
		$(CXX) -o $@ $< -ldl

# todo: qmake .pro file
CXXFLAGS	+= -I/usr/include/qt -I/usr/include/qt/QtCore -fPIC -std=c++11

plugincandidates:	plugincandidates.o
		$(CXX) -o $@ $< -lQt5Core -ldl

clean:
		rm helper.o plugincandidates.o

