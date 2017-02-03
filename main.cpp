#include <process.hpp>
#include <asar.hpp>
#include <iostream>

// struct argument_parser {

// private:
//     std::vector<std::string>
// };

int main() {
    for(auto&& p : bd::enumerate_processes()) {
        try {
            std::cout << "[PID: " << p.id << "]: " << p.exe().string() << '\n';
        }
        catch(bd::process_error& e) {
            std::cerr << "Could not fetch info for PID " << p.id << '\n';
        }
    }
}
