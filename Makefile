COMMAND = g++ -std=c++17 -pthread -Wall -Wextra -O2
SERVER_SRC = server.cpp ThreadPool.cpp
CLIENT_SRC = client.cpp
STRESS_SRC = client_stress.cpp

all: server client stress

server: server.cpp
	mkdir -p dist
	$(COMMAND) -o dist/server $(SERVER_SRC)

client: client.cpp
	mkdir -p dist
	$(COMMAND) -o dist/client $(CLIENT_SRC)

stress: client.cpp
	mkdir -p dist
	$(COMMAND) -o dist/stress $(STRESS_SRC)

clean:
	rm -rf dist
