#ifndef OPENSSL_INIT_H
#define OPENSSL_INIT_H

#include <openssl/ssl.h>
#include <openssl/err.h>

void init_openssl();
void cleanup_openssl();

#endif // OPENSSL_INIT_H
