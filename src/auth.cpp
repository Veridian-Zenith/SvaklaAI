#include <iostream>
#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include "../include/openssl_init.h"

void generate_key_pair() {
    RSA *rsa = RSA_generate_key(2048, RSA_F4, NULL, NULL);
    if (!rsa) {
        std::cerr << "Error generating RSA key pair" << std::endl;
        return;
    }

    BIO *bp_public = BIO_new_file("public.pem", "w+");
    BIO *bp_private = BIO_new_file("private.pem", "w+");

    PEM_write_bio_RSAPublicKey(bp_public, rsa);
    PEM_write_bio_RSAPrivateKey(bp_private, rsa, NULL, NULL, 0, NULL, NULL);

    BIO_free_all(bp_public);
    BIO_free_all(bp_private);
    RSA_free(rsa);
}

bool authenticate_user(const std::string &public_key_path, const std::string &private_key_path) {
    // Authentication logic here
    return true;
}
