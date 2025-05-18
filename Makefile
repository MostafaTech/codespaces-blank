COMMAND = g++ -std=c++17 -pthread -Wall -Wextra -O2
SERVER_SRC = server/main.cpp server/ThreadPool.cpp
CLIENT_SRC = clients/client.cpp
STRESS_SRC = clients/client_stress.cpp

.PHONY: server

all: server client stress

server:
	mkdir -p dist
	$(COMMAND) -o dist/server $(SERVER_SRC)

client:
	mkdir -p dist
	$(COMMAND) -o dist/client $(CLIENT_SRC)

stress:
	mkdir -p dist
	$(COMMAND) -o dist/stress $(STRESS_SRC)

clean:
	rm -rf dist
