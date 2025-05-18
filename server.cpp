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

std::string handleCommand(const std::string& input) {
    auto tokens = split(input);
    if (tokens.empty()) return "ERR empty command\n";

    std::string cmd = tokens[0];

    if (cmd == "SET") {
        if (tokens.size() < 3) return "ERR usage: SET key value\n";
        std::lock_guard<std::mutex> lock(db_mutex);
        db[tokens[1]] = tokens[2];
        return "OK\n";
    }
    else if (cmd == "GET") {
        if (tokens.size() != 2) return "ERR usage: GET key\n";
        std::lock_guard<std::mutex> lock(db_mutex);
        auto it = db.find(tokens[1]);
        if (it == db.end()) return "ERR key not found\n";
        return it->second + "\n";
    }
    else if (cmd == "DEL") {
        if (tokens.size() != 2) return "ERR usage: DEL key\n";
        std::lock_guard<std::mutex> lock(db_mutex);
        if (db.erase(tokens[1]) == 0)
            return "ERR key not found\n";
        return "OK\n";
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
        std::string input(buffer);
        std::string response = handleCommand(input);

        send(client_socket, response.c_str(), response.size(), 0);
        memset(buffer, 0, BUFFER_SIZE);
    }

    std::cout << "Client disconnected.\n";
    close(client_socket);
}

int main() {
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

    std::cout << "Server listening on port " << PORT << "...\n";

    while (true) {
        int client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (client_socket < 0) {
            perror("accept");
            continue;
        }

        std::cout << "New client connected.\n";

        std::thread clientThread(handleClient, client_socket);
        clientThread.detach();
    }

    return 0;
}
