#include <iostream>
#include <string>
#include <fstream>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include "../include/openssl_init.h"

void save_context(const std::string &filename, const std::string &context) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << context;
        file.close();
    } else {
        std::cerr << "Unable to open file for writing" << std::endl;
    }
}

std::string load_context(const std::string &filename) {
    std::ifstream file(filename);
    std::string context;
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            context += line + "\n";
        }
        file.close();
    } else {
        std::cerr << "Unable to open file for reading" << std::endl;
    }
    return context;
}
