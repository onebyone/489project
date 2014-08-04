CC=g++
FLAGS=-lpthread -std=c++11
all: run_client

run_client:
	rm -f ./client/client
	$(CC) ./client/*.cpp -o ./client/client $(FLAGS)
	cd ./client; ./client

run_server:
	rm -f ./server/server
	$(CC) ./server/*.cpp -o ./server/server $(FLAGS)
	cd ./server; ./server

clean:
	rm -f ./client/client
	rm -f ./client/torrent_list
	rm -f ./server/server
	rm -f ./server/*.peers
	rm -f ./server/*.torrent
	rm -f ./server/torrent_list

