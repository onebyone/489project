CC=g++
FLAGS=-lpthread -std=c++11

all: client

client:
	$(CC) ./client/*.cpp -o ./client/client $(FLAGS)

server:
	$(CC) ./server/*.cpp -o ./server/server $(FLAGS)

clean:
	rm -f ./client/client
	rm -f ./server/server

