#include <iostream>
#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include "../include/openssl_init.h"

void handle_client(SSL *ssl) {
    const char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
                           "<html><body><h1>SvaklaAI Control Panel</h1></body></html>";
    SSL_write(ssl, response, strlen(response));
    SSL_shutdown(ssl);
    SSL_free(ssl);
}

void start_server(int port) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error binding socket" << std::endl;
        close(server_socket);
        return;
    }

    if (listen(server_socket, 3) < 0) {
        std::cerr << "Error listening on socket" << std::endl;
        close(server_socket);
        return;
    }

    std::cout << "Web interface server listening on port " << port << std::endl;

    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) {
        std::cerr << "Error creating SSL context" << std::endl;
        close(server_socket);
        return;
    }

    if (SSL_CTX_use_certificate_file(ctx, "/etc/letsencrypt/live/vz.strangled.net/fullchain.pem", SSL_FILETYPE_PEM) <= 0) {
        std::cerr << "Error loading certificate" << std::endl;
        SSL_CTX_free(ctx);
        close(server_socket);
        return;
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "/etc/letsencrypt/live/vz.strangled.net/privkey.pem", SSL_FILETYPE_PEM) <= 0) {
        std::cerr << "Error loading private key" << std::endl;
        SSL_CTX_free(ctx);
        close(server_socket);
        return;
    }

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            std::cerr << "Error accepting connection" << std::endl;
            continue;
        }

        SSL *ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client_socket);

        if (SSL_accept(ssl) <= 0) {
            std::cerr << "Error accepting SSL connection" << std::endl;
            SSL_free(ssl);
            close(client_socket);
            continue;
        }

        handle_client(ssl);
    }

    SSL_CTX_free(ctx);
    close(server_socket);
}
