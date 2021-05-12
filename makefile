CXX = g++
CC = gcc
OBJ = SQLFunction.o interrupt_vs_grid.o 
FLAG = -g -Wall -o interrupt_vs_grid
LIB = -lmysqlclient -lpthread -lglpk
INCLUDE = -I/usr/include/mysql -L/usr/lib/mysql 

all: $(OBJ)
	$(CXX) $(OBJ) $(FLAG) $(LIB)

%.o: %.cpp
	$(CXX) -c $< $(INCLUDE) 

clean:
	rm -rf *.o
	rm -rf *.out

rebuild:
	$(MAKE) clean
	$(MAKE) all



                          
