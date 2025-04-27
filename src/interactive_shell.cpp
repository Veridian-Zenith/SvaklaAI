#include <iostream>
#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "../include/openssl_init.h"

void display_menu() {
    std::cout << "Interactive Shell Menu:" << std::endl;
    std::cout << "1. Start HTTP Server" << std::endl;
    std::cout << "2. Start WebSocket Server" << std::endl;
    std::cout << "3. Start API Server" << std::endl;
    std::cout << "4. Start Web Interface Server" << std::endl;
    std::cout << "5. Start External Service Interface" << std::endl;
    std::cout << "6. Authenticate User" << std::endl;
    std::cout << "7. Run AI Engine" << std::endl;
    std::cout << "8. Load Plugins" << std::endl;
    std::cout << "9. Manage Local Memory" << std::endl;
    std::cout << "10. Exit" << std::endl;
}

void handle_choice(int choice) {
    switch (choice) {
        case 1:
            extern void start_http_server();
            start_http_server();
            break;
        case 2:
            extern void start_websocket_server();
            start_websocket_server();
            break;
        case 3:
            extern void start_api_server();
            start_api_server();
            break;
        case 4:
            extern void start_web_interface_server();
            start_web_interface_server();
            break;
        case 5:
            extern void start_external_service_interface();
            start_external_service_interface();
            break;
        case 6:
            extern void authenticate_user();
            authenticate_user();
            break;
        case 7:
            extern void run_ai_engine();
            run_ai_engine();
            break;
        case 8:
            extern void load_plugins();
            load_plugins();
            break;
        case 9:
            extern void manage_local_memory();
            manage_local_memory();
            break;
        case 10:
            std::cout << "Exiting..." << std::endl;
            break;
        default:
            std::cout << "Invalid choice. Please try again." << std::endl;
    }
}
