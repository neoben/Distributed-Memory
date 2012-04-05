CC = g++
CFLAG1 = -Wall
CFLAG2 = -lpthread
SOURCES1 = ./client1.cpp ../src/client_library.cpp
SOURCES2 = ./client2.cpp ../src/client_library.cpp
OUT_EXE1 = client1
OUT_EXE2 = client2

build : $(SOURCES1)
	$(CC) $(CFLAG1) $(CFLAG2) -I ../src/ $(SOURCES1) -o $(OUT_EXE1)
	$(CC) $(CFLAG1) $(CFLAG2) -I ../src/ $(SOURCES2) -o $(OUT_EXE2)

clean : 
	rm -f *.o core

rebuild : 
	clean build


