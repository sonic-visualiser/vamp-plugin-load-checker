
CXXFLAGS	:= -Wall -Werror 

checker:	checker.o
		$(CXX) -o $@ $< -ldl

clean:
		rm checker.o

