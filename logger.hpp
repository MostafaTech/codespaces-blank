#pragma once
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

inline std::string current_time() {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto t_c = system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&t_c), "%H:%M:%S");
    return oss.str();
}

inline void log(const std::string& msg) {
    std::cout << "[" << current_time() << "] " << msg << std::endl;
}
