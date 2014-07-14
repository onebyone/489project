CC=g++
FLAGS=-lpthread -std=c++11

all: g++_client g++_server g++_torrent	

g++_client:
	$(CC) ./client/*.cpp -o ./client/client $(FLAGS)

g++_server:
	$(CC) ./server/*.cpp -o ./server/server $(FLAGS)

g++_torrent:
	$(CC) ./create_torrent/*.cpp -o ./create_torrent/create_torrent $(FLAGS)

clean:
	rm -f ./create_torrent/create_torrent
	rm -f ./client/client
	rm -f ./server/server
	rm -f ./create_torrent/*~
	rm -f ./client/*~
	rm -f ./server/*~
	rm -f ./*~
