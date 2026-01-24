/*
 * Copyright 2025 Guillaume Guillet
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "private/fge_crypt.hpp"
#include "FastEngine/network/C_packet.hpp"
#include <iostream>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include "private/fge_debug.hpp"

namespace fge::priv
{

namespace
{

bool CryptGenerateKeyAndCertificate(EVP_PKEY*& privateKey, X509*& certificate)
{
    // Generate RSA key using EVP_PKEY and EVP_PKEY_CTX
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (ctx == nullptr)
    {
        std::cerr << "Failed to create EVP_PKEY_CTX" << std::endl;
        return false;
    }

    if (EVP_PKEY_keygen_init(ctx) <= 0)
    {
        std::cerr << "Failed to initialize keygen context" << std::endl;
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0)
    {
        std::cerr << "Failed to set RSA key length" << std::endl;
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    if (EVP_PKEY_keygen(ctx, &privateKey) <= 0)
    {
        std::cerr << "Failed to generate RSA key" << std::endl;
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    EVP_PKEY_CTX_free(ctx);

    // Generate X509 certificate
    certificate = X509_new();
    if (certificate == nullptr)
    {
        std::cerr << "Failed to create X509 certificate" << std::endl;
        EVP_PKEY_free(privateKey);
        return false;
    }

    X509_set_version(certificate, 2);
    ASN1_INTEGER_set(X509_get_serialNumber(certificate), 1);
    X509_gmtime_adj(X509_get_notBefore(certificate), 0);
    //Validity only for a day
    X509_gmtime_adj(X509_get_notAfter(certificate), 24 * 3600);
    X509_set_pubkey(certificate, privateKey);

    // Set certificate subject and issuer name
    X509_NAME* name = X509_get_subject_name(certificate);
    X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, reinterpret_cast<unsigned char const*>("CH"), -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC, reinterpret_cast<unsigned char const*>("FGE"), -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
                               reinterpret_cast<unsigned char const*>("https://github.com/JonathSpirit/FastEngine.git"),
                               -1, -1, 0);
    X509_set_issuer_name(certificate, name);

    // Sign the certificate with the key
    if (X509_sign(certificate, privateKey, EVP_sha256()) == 0)
    {
        EVP_PKEY_free(privateKey);
        X509_free(certificate);
        return false;
    }

    return true;
}

} //end namespace

bool CryptClientInit(void*& ctx)
{
    // Initialize OpenSSL DTLS context
    SSL_library_init();
    SSL_load_error_strings();

    auto const* method = DTLS_client_method();
    ctx = SSL_CTX_new(method);
    if (ctx == nullptr)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }
    SSL_CTX_set_options(static_cast<SSL_CTX*>(ctx), SSL_OP_NO_QUERY_MTU);
    SSL_CTX_set_options(static_cast<SSL_CTX*>(ctx), SSL_OP_NO_COMPRESSION);
    SSL_CTX_set_options(static_cast<SSL_CTX*>(ctx), SSL_OP_NO_RENEGOTIATION);

    char const* cipher_list = "ECDHE-RSA-AES256-GCM-SHA384";
    if (SSL_CTX_set_cipher_list(static_cast<SSL_CTX*>(ctx), cipher_list) != 1)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }

    EVP_PKEY* privateKey = nullptr;
    X509* certificate = nullptr;
    if (!CryptGenerateKeyAndCertificate(privateKey, certificate))
    {
        return false;
    }

    // Load the certificate file
    if (SSL_CTX_use_certificate(static_cast<SSL_CTX*>(ctx), certificate) <= 0)
    {
        ERR_print_errors_fp(stderr);
        EVP_PKEY_free(privateKey);
        X509_free(certificate);
        return false;
    }

    // Load the private key file
    if (SSL_CTX_use_PrivateKey(static_cast<SSL_CTX*>(ctx), privateKey) <= 0)
    {
        ERR_print_errors_fp(stderr);
        EVP_PKEY_free(privateKey);
        X509_free(certificate);
        return false;
    }

    EVP_PKEY_free(privateKey);
    X509_free(certificate);

    // Verify that the private key matches the certificate
    if (SSL_CTX_check_private_key(static_cast<SSL_CTX*>(ctx)) <= 0)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }

    return true;
}
bool CryptServerInit(void*& ctx)
{
    // Initialize OpenSSL DTLS context
    SSL_library_init();
    SSL_load_error_strings();

    auto const* method = DTLS_server_method();
    ctx = SSL_CTX_new(method);
    if (ctx == nullptr)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }
    SSL_CTX_set_options(static_cast<SSL_CTX*>(ctx), SSL_OP_NO_QUERY_MTU);
    SSL_CTX_set_options(static_cast<SSL_CTX*>(ctx), SSL_OP_NO_COMPRESSION);
    SSL_CTX_set_options(static_cast<SSL_CTX*>(ctx), SSL_OP_NO_RENEGOTIATION);

    char const* cipher_list = "ECDHE-RSA-AES256-GCM-SHA384";
    if (SSL_CTX_set_cipher_list(static_cast<SSL_CTX*>(ctx), cipher_list) != 1)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }

    EVP_PKEY* privateKey = nullptr;
    X509* certificate = nullptr;
    if (!CryptGenerateKeyAndCertificate(privateKey, certificate))
    {
        return false;
    }

    // Load the certificate file
    if (SSL_CTX_use_certificate(static_cast<SSL_CTX*>(ctx), certificate) <= 0)
    {
        ERR_print_errors_fp(stderr);
        EVP_PKEY_free(privateKey);
        X509_free(certificate);
        return false;
    }

    // Load the private key file
    if (SSL_CTX_use_PrivateKey(static_cast<SSL_CTX*>(ctx), privateKey) <= 0)
    {
        ERR_print_errors_fp(stderr);
        EVP_PKEY_free(privateKey);
        X509_free(certificate);
        return false;
    }

    EVP_PKEY_free(privateKey);
    X509_free(certificate);

    // Verify that the private key matches the certificate
    if (SSL_CTX_check_private_key(static_cast<SSL_CTX*>(ctx)) <= 0)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }

    return true;
}
void CryptUninit(void*& ctx)
{
    if (ctx != nullptr)
    {
        SSL_CTX_free(static_cast<SSL_CTX*>(ctx));
        ctx = nullptr;
    }
}

bool CryptClientCreate(void* ctx, net::CryptInfo& client)
{
    CryptClientDestroy(client);

    // Create an SSL object and memory BIOs
    SSL* ssl = SSL_new(static_cast<SSL_CTX*>(ctx));
    if (ssl == nullptr)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }
    client._ssl = ssl;

    BIO* rbio = BIO_new(BIO_s_mem());
    BIO* wbio = BIO_new(BIO_s_mem());
    if (rbio == nullptr || wbio == nullptr)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }
    client._rbio = rbio;
    client._wbio = wbio;

    // Tell the memory BIOs to return -1 when no data is available
    BIO_set_mem_eof_return(rbio, -1);
    BIO_set_mem_eof_return(wbio, -1);

    // Attach the BIO pair to the SSL object (After this call, SSL owns the BIOs)
    SSL_set_bio(ssl, rbio, wbio);

    // Set the SSL object to “connect” (client) state.
    SSL_set_connect_state(ssl);

    // We handle the MTU
    SSL_set_mtu(ssl, std::numeric_limits<uint16_t>::max());
    DTLS_set_link_mtu(ssl, std::numeric_limits<uint16_t>::max());

    return true;
}
bool CryptServerCreate(void* ctx, net::CryptInfo& client)
{
    CryptClientDestroy(client);

    // Create an SSL object and memory BIOs
    SSL* ssl = SSL_new(static_cast<SSL_CTX*>(ctx));
    if (ssl == nullptr)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }
    client._ssl = ssl;

    BIO* rbio = BIO_new(BIO_s_mem());
    BIO* wbio = BIO_new(BIO_s_mem());
    if (rbio == nullptr || wbio == nullptr)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }
    client._rbio = rbio;
    client._wbio = wbio;

    // Tell the memory BIOs to return -1 when no data is available
    BIO_set_mem_eof_return(rbio, -1);
    BIO_set_mem_eof_return(wbio, -1);

    // Attach the BIO pair to the SSL object (After this call, SSL owns the BIOs)
    SSL_set_bio(ssl, rbio, wbio);

    // Set the SSL object to “accept” (server) state.
    SSL_set_accept_state(ssl);

    // We handle the MTU
    SSL_set_mtu(ssl, std::numeric_limits<uint16_t>::max());
    DTLS_set_link_mtu(ssl, std::numeric_limits<uint16_t>::max());

    return true;
}
void CryptClientDestroy(net::CryptInfo& client)
{
    if (client._ssl == nullptr)
    {
        return;
    }

    SSL_shutdown(static_cast<SSL*>(client._ssl));
    SSL_free(static_cast<SSL*>(client._ssl));
    client._ssl = nullptr;
    client._rbio = nullptr;
    client._wbio = nullptr;
}

bool CryptEncrypt(net::Client& client, net::Packet& packet)
{
    if (packet.getDataSize() > 16384)
    {
        //TODO: see C_socket.hpp, records can be up to 16384 bytes
        FGE_DEBUG_PRINT("CryptEncrypt: packet size is bigger than a DTLS record (16384 bytes) !");
        return false;
    }

    auto& info = client.getCryptInfo();

    if (SSL_write(static_cast<SSL*>(info._ssl), packet.getData(), packet.getDataSize()) <= 0)
    {
        return false;
    }

    packet.clear();
    auto const pendingSize = BIO_ctrl_pending(static_cast<BIO*>(info._wbio));
    if (pendingSize == 0)
    {
        return false;
    }
    packet.append(pendingSize);

    if (BIO_read(static_cast<BIO*>(info._wbio), packet.getData(), pendingSize) < 0)
    {
        return false;
    }
    return true;
}
bool CryptDecrypt(net::Client& client, net::Packet& packet)
{
    auto& info = client.getCryptInfo();

    if (BIO_write(static_cast<BIO*>(info._rbio), packet.getData(), packet.getDataSize()) <= 0)
    {
        return false;
    }

    auto const result = SSL_read(static_cast<SSL*>(info._ssl), packet.getData(), packet.getDataSize());
    if (result <= 0)
    {
        return false;
    }

    packet.shrink(packet.getDataSize() - result);
    return true;
}

} // namespace fge::priv
