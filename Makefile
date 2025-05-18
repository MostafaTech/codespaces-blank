COMMAND = g++ -std=c++17 -pthread -Wall -Wextra -O2

all: server client

server: server.cpp
	mkdir -p dist
	$(COMMAND) -o dist/server server.cpp

client: client.cpp
	mkdir -p dist
	$(COMMAND) -o dist/client client.cpp

clean:
	rm -rf dist
