#include "web_interface.h"
#include <iostream>
#include <openssl/ssl.h>
#include <openssl/err.h>

void initializeWebInterface() {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    std::cout << "Web interface initialized." << std::endl;
}

void handleWebRequest() {
    std::cout << "Handling web request." << std::endl;
}
