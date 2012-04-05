CC = g++
CFLAG1 = -Wall
CFLAG2 = -lpthread
SOURCES = ./server.cpp
OUT_EXE = server

build : $(SOURCES)
	$(CC) $(CFLAG1) $(CFLAG2) -I ../src/ $(SOURCES) -o $(OUT_EXE)

clean : 
	rm -f *.o core

rebuild : 
	clean build
	
