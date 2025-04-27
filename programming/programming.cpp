#include "programming.h"
#include <iostream>

void initializeShell() {
    std::cout << "Interactive shell initialized." << std::endl;
}

void executeCommand(const std::string& command) {
    std::cout << "Executing command: " << command << std::endl;
}
