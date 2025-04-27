#include "chat_storage.h"
#include <iostream>
#include <fstream>
#include <string>

void initializeChatStorage() {
    std::cout << "Chat storage initialized." << std::endl;
}

void saveChat(const std::string& chatName, const std::string& chatContent) {
    std::ofstream outFile(chatName + ".txt");
    if (outFile.is_open()) {
        outFile << chatContent;
        outFile.close();
        std::cout << "Chat saved: " << chatName << std::endl;
    } else {
        std::cerr << "Unable to open file: " << chatName << std::endl;
    }
}

std::string loadChat(const std::string& chatName) {
    std::ifstream inFile(chatName + ".txt");
    std::string content;
    if (inFile.is_open()) {
        std::string line;
        while (std::getline(inFile, line)) {
            content += line + "\n";
        }
        inFile.close();
        std::cout << "Chat loaded: " << chatName << std::endl;
    } else {
        std::cerr << "Unable to open file: " << chatName << std::endl;
    }
    return content;
}
