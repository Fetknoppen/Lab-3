all: test client server


main.o: main.c
	$(CXX) -Wall -I. -c main.c


test: main.o
	$(CXX) -I./ -Wall  -o test main.o 


client: client.o
	$(CXX) -Wall -o cchat client.o

server: server.o
	$(CXX) -Wall -o cserverd server.o


clean:
	rm *.o test cchat cserverd
