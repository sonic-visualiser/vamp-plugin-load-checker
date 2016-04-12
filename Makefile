
CXXFLAGS	:= -Wall -Werror 

helper:	helper.o
		$(CXX) -o $@ $< -ldl

clean:
		rm helper.o

