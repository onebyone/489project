CC=g++
FLAGS=-lpthread -std=c++11

all: g++_client g++_server g++_torrent	

g++_client:
	$(CC) ./client/*.cpp -o ./client/client $(FLAGS)

g++_server:
	$(CC) ./server/*.cpp -o ./server/server $(FLAGS)

clean:
	rm -f ./client/client
	rm -f ./server/server

