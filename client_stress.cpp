#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void send_commands(int id, int num_requests) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "[Thread " << id << "] Socket creation error\n";
        return;
    }

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(4000);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "[Thread " << id << "] Invalid address\n";
        close(sock);
        return;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "[Thread " << id << "] Connection failed\n";
        close(sock);
        return;
    }

    for (int i = 0; i < num_requests; ++i) {
        std::string key = "key" + std::to_string(i);
        std::string value = "val" + std::to_string(i);
        std::string set_cmd = "SET " + key + " " + value + "\n";
        send(sock, set_cmd.c_str(), set_cmd.length(), 0);

        char buffer[1024] = {0};
        int bytes = read(sock, buffer, sizeof(buffer));
        std::cout << "[Thread " << id << "] Response: " << std::string(buffer, bytes);
    }

    close(sock);
}

int main() {
    const int thread_count = 10;
    const int requests_per_thread = 50;

    std::vector<std::thread> threads;

    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back(send_commands, i, requests_per_thread);
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "Done stress testing.\n";
    return 0;
}
