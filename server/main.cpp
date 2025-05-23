#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include "logger.hpp"
#include "ThreadPool.hpp"

#define PORT 4000
#define BUFFER_SIZE 1024

std::map<std::string, std::string> db;
std::mutex db_mutex;

std::vector<std::string> split(const std::string& line) {
    std::istringstream iss(line);
    std::string word;
    std::vector<std::string> result;
    while (iss >> word) result.push_back(word);
    return result;
}

std::string handleCommand(const std::string& input, bool& should_end) {
    auto tokens = split(input);
    if (tokens.empty()) return "ERR empty command\n";

    std::string cmd = tokens[0];

    if (cmd == "ALL") {
        std::lock_guard<std::mutex> lock(db_mutex);
        std::string result = "";
        for (auto i : db) {
            result += i.first + " ";
        }
        return result + "\n";
    }
    else if (cmd == "SET" || cmd == "SETE") {
        if (tokens.size() < 3) return "ERR usage: SET key value\n";
        std::lock_guard<std::mutex> lock(db_mutex);
        db[tokens[1]] = tokens[2];
        if (cmd == "SETE") should_end = true;
        return "OK\n";
    }
    else if (cmd == "GET" || cmd == "GETE") {
        if (tokens.size() != 2) return "ERR usage: GET key\n";
        std::lock_guard<std::mutex> lock(db_mutex);
        auto it = db.find(tokens[1]);
        if (it == db.end()) return "ERR key not found\n";
        if (cmd == "GETE") should_end = true;
        return it->second + "\n";
    }
    else if (cmd == "DEL" || cmd == "DELE") {
        if (tokens.size() != 2) return "ERR usage: DEL key\n";
        std::lock_guard<std::mutex> lock(db_mutex);
        if (db.erase(tokens[1]) == 0)
            return "ERR key not found\n";
        if (cmd == "DELE") should_end = true;
        return "OK\n";
    }
    else if (cmd == "END") {
        should_end = true;
        return "BYE\n";
    }
    else {
        return "ERR unknown command\n";
    }
}

void handleClient(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};

    while (true) {
        int valread = read(client_socket, buffer, BUFFER_SIZE - 1);
        if (valread <= 0) break;

        buffer[valread] = '\0';
        bool should_end = false;
        std::string input(buffer);
        std::string response = handleCommand(input, should_end);

        send(client_socket, response.c_str(), response.size(), 0);
        memset(buffer, 0, BUFFER_SIZE);
        
        if (should_end) break;
    }

    log("Client " + std::to_string(client_socket) + " disconnected");
    close(client_socket);
}

int main() {
    ThreadPool pool(4);

    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return 1;
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        return 1;
    }

    log("Server listening on port " + std::to_string(PORT) + "...\n");

    while (true) {
        int client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (client_socket < 0) continue;

        log("New client connected: socket=" + std::to_string(client_socket));
        pool.enqueue([client_socket]() {
            handleClient(client_socket);
        });
    }

    return 0;
}
