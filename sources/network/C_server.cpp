/*
 * Copyright 2024 Guillaume Guillet
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

#include "FastEngine/network/C_server.hpp"
#include "FastEngine/C_alloca.hpp"
#include "FastEngine/manager/network_manager.hpp"
#include <iostream>
#include <openssl/err.h>
#include <openssl/ssl.h>

namespace fge::net
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

[[nodiscard]] bool CryptClientInit(SSL_CTX*& ctx)
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
    SSL_CTX_set_options(ctx, SSL_OP_NO_QUERY_MTU);
    //SSL_CTX_set_options(ctx, SSL_OP_NO_COMPRESSION);

    char const* cipher_list = "ECDHE-RSA-AES256-GCM-SHA384";
    if (SSL_CTX_set_cipher_list(ctx, cipher_list) != 1)
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
    if (SSL_CTX_use_certificate(ctx, certificate) <= 0)
    {
        ERR_print_errors_fp(stderr);
        EVP_PKEY_free(privateKey);
        X509_free(certificate);
        return false;
    }

    // Load the private key file
    if (SSL_CTX_use_PrivateKey(ctx, privateKey) <= 0)
    {
        ERR_print_errors_fp(stderr);
        EVP_PKEY_free(privateKey);
        X509_free(certificate);
        return false;
    }

    EVP_PKEY_free(privateKey);
    X509_free(certificate);

    // Verify that the private key matches the certificate
    if (SSL_CTX_check_private_key(ctx) <= 0)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }

    return true;
}
[[nodiscard]] bool CryptServerInit(SSL_CTX*& ctx)
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
    SSL_CTX_set_options(ctx, SSL_OP_NO_QUERY_MTU);
    //SSL_CTX_set_options(ctx, SSL_OP_NO_COMPRESSION);

    char const* cipher_list = "ECDHE-RSA-AES256-GCM-SHA384";
    if (SSL_CTX_set_cipher_list(ctx, cipher_list) != 1)
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
    if (SSL_CTX_use_certificate(ctx, certificate) <= 0)
    {
        ERR_print_errors_fp(stderr);
        EVP_PKEY_free(privateKey);
        X509_free(certificate);
        return false;
    }

    // Load the private key file
    if (SSL_CTX_use_PrivateKey(ctx, privateKey) <= 0)
    {
        ERR_print_errors_fp(stderr);
        EVP_PKEY_free(privateKey);
        X509_free(certificate);
        return false;
    }

    EVP_PKEY_free(privateKey);
    X509_free(certificate);

    // Verify that the private key matches the certificate
    if (SSL_CTX_check_private_key(ctx) <= 0)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }

    return true;
}
void CryptUninit(SSL_CTX*& ctx)
{
    if (ctx != nullptr)
    {
        SSL_CTX_free(ctx);
        ctx = nullptr;
    }
}

[[nodiscard]] bool CryptClientCreate(SSL_CTX* ctx, Client& client)
{
    // Create an SSL object and memory BIOs
    SSL* ssl = SSL_new(ctx);
    if (ssl == nullptr)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }
    client.getCryptInfo()._ssl = ssl;

    BIO* rbio = BIO_new(BIO_s_mem());
    BIO* wbio = BIO_new(BIO_s_mem());
    if (rbio == nullptr || wbio == nullptr)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }
    client.getCryptInfo()._rbio = rbio;
    client.getCryptInfo()._wbio = wbio;

    // Tell the memory BIOs to return -1 when no data is available
    BIO_set_mem_eof_return(rbio, -1);
    BIO_set_mem_eof_return(wbio, -1);

    // Attach the BIO pair to the SSL object (After this call, SSL owns the BIOs)
    SSL_set_bio(ssl, rbio, wbio);

    // Set the SSL object to “connect” (client) state.
    SSL_set_connect_state(ssl);

    // We handle the MTU
    SSL_set_mtu(ssl, std::numeric_limits<uint16_t>::max());

    return true;
}
[[nodiscard]] bool CryptServerCreate(SSL_CTX* ctx, Client& client)
{
    // Create an SSL object and memory BIOs
    SSL* ssl = SSL_new(ctx);
    if (ssl == nullptr)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }
    client.getCryptInfo()._ssl = ssl;

    BIO* rbio = BIO_new(BIO_s_mem());
    BIO* wbio = BIO_new(BIO_s_mem());
    if (rbio == nullptr || wbio == nullptr)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }
    client.getCryptInfo()._rbio = rbio;
    client.getCryptInfo()._wbio = wbio;

    // Tell the memory BIOs to return -1 when no data is available
    BIO_set_mem_eof_return(rbio, -1);
    BIO_set_mem_eof_return(wbio, -1);

    // Attach the BIO pair to the SSL object (After this call, SSL owns the BIOs)
    SSL_set_bio(ssl, rbio, wbio);

    // Set the SSL object to “accept” (server) state.
    SSL_set_accept_state(ssl);

    // We handle the MTU
    SSL_set_mtu(ssl, std::numeric_limits<uint16_t>::max());

    return true;
}
void CryptClientDestroy(Client& client)
{
    auto& info = client.getCryptInfo();

    if (info._ssl == nullptr)
    {
        return;
    }

    SSL_shutdown(static_cast<SSL*>(info._ssl));
    SSL_free(static_cast<SSL*>(info._ssl));
    info._ssl = nullptr;
    info._rbio = nullptr;
    info._wbio = nullptr;
}

} // namespace

//ServerFluxUdp
NetFluxUdp::~NetFluxUdp()
{
    this->clearPackets();
}

void NetFluxUdp::clearPackets()
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    this->_g_packets.clear();
    this->_g_remainingPackets = 0;
}
bool NetFluxUdp::pushPacket(ReceivedPacketPtr&& fluxPck)
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    if (this->_g_packets.size() >= this->g_maxPackets)
    {
        return false;
    }
    this->_g_packets.push_back(std::move(fluxPck));
    return true;
}
void NetFluxUdp::forcePushPacket(ReceivedPacketPtr fluxPck)
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    this->_g_packets.push_back(std::move(fluxPck));
}
void NetFluxUdp::forcePushPacketFront(ReceivedPacketPtr fluxPck)
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    this->_g_packets.push_front(std::move(fluxPck));
}

FluxProcessResults NetFluxUdp::processReorder(Client& client,
                                              ReceivedPacketPtr& packet,
                                              ProtocolPacket::CounterType currentCounter,
                                              bool ignoreRealm)
{
    auto const currentRealm = ignoreRealm ? packet->retrieveRealm().value() : client.getCurrentRealm();

    if (client.getPacketReorderer().isEmpty())
    {
        auto const stat = PacketReorderer::checkStat(packet, currentCounter, currentRealm);

        if (stat == PacketReorderer::Stats::RETRIEVABLE)
        {
            return FluxProcessResults::RETRIEVABLE;
        }

        client.getPacketReorderer().push(std::move(packet));
        return FluxProcessResults::NOT_RETRIEVABLE;
    }

    //We push the packet in the reorderer
    client.getPacketReorderer().push(std::move(packet));

    //At this point we are sure that the reorderer contains at least 2 packets

    bool forced = client.getPacketReorderer().isForced();
    auto const stat = client.getPacketReorderer().checkStat(currentCounter, currentRealm).value();
    if (!forced && stat != PacketReorderer::Stats::RETRIEVABLE)
    {
        return FluxProcessResults::NOT_RETRIEVABLE;
    }

    packet = client.getPacketReorderer().pop();
    packet->addFlags(FGE_NET_HEADER_LOCAL_REORDERED_FLAG);

    std::size_t containerInversedSize = 0;
    auto* containerInversed = FGE_ALLOCA_T(ReceivedPacketPtr, FGE_NET_PACKET_REORDERER_CACHE_MAX);
    FGE_PLACE_CONSTRUCT(ReceivedPacketPtr, FGE_NET_PACKET_REORDERER_CACHE_MAX, containerInversed);

    while (auto const stat = client.getPacketReorderer().checkStat(currentCounter, currentRealm))
    {
        if (!(stat == PacketReorderer::Stats::RETRIEVABLE || forced))
        {
            break;
        }

        auto reorderedPacket = client.getPacketReorderer().pop();

        //Add the LOCAL_REORDERED_FLAG to the header
        reorderedPacket->addFlags(FGE_NET_HEADER_LOCAL_REORDERED_FLAG);

        //Add it to the container (we are going to push in front of the flux queue, so we need to inverse the order)
        containerInversed[containerInversedSize++] = std::move(reorderedPacket);
    }

    //Now we push the packets (until the last one) in the correct order in the flux queue
    for (std::size_t i = containerInversedSize; i != 0; --i)
    {
        this->forcePushPacketFront(std::move(containerInversed[i - 1]));
        ++this->_g_remainingPackets;
    }

    FGE_PLACE_DESTRUCT(ReceivedPacketPtr, FGE_NET_PACKET_REORDERER_CACHE_MAX, containerInversed);

    return FluxProcessResults::RETRIEVABLE;
}

ReceivedPacketPtr NetFluxUdp::popNextPacket()
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    if (!this->_g_packets.empty())
    {
        ReceivedPacketPtr tmpPck = std::move(this->_g_packets.front());
        this->_g_packets.pop_front();
        return tmpPck;
    }
    return nullptr;
}
std::size_t NetFluxUdp::getPacketsSize() const
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    return this->_g_packets.size();
}
bool NetFluxUdp::isEmpty() const
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    return this->_g_packets.empty();
}

void NetFluxUdp::setMaxPackets(std::size_t n)
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    this->g_maxPackets = n;
}
std::size_t NetFluxUdp::getMaxPackets() const
{
    std::scoped_lock const lock(this->_g_mutexFlux);
    return this->g_maxPackets;
}

//ServerNetFluxUdp
void ServerNetFluxUdp::processClients()
{
    auto const now = std::chrono::steady_clock::now();
    this->g_commandsUpdateTick +=
            std::min(std::chrono::milliseconds{1},
                     std::chrono::duration_cast<std::chrono::milliseconds>(now - this->g_lastCommandUpdateTimePoint));

    auto clientsLock = this->_clients.acquireLock();

    for (auto it = this->_clients.begin(clientsLock); it != this->_clients.end(clientsLock);)
    {
        auto& client = it->second._client;

        //Handle timeout
        if (client->getStatus().getNetworkStatus() == ClientStatus::NetworkStatus::TIMEOUT)
        {
            continue;
        }

        if (client->getStatus().isTimeout())
        {
            client->getStatus().setNetworkStatus(ClientStatus::NetworkStatus::TIMEOUT);
            client->clearPackets();

            auto tmpIdentity = it->first;
            auto tmpClient = std::move(it->second._client);
            it = this->_clients.remove(it, clientsLock);
            this->_onClientTimeout.call(tmpClient, tmpIdentity);
            continue;
        }

        //Handle commands
        if (this->g_commandsUpdateTick >= FGE_NET_CMD_UPDATE_TICK_MS)
        {
            auto& commands = it->second._commands;
            if (!commands.empty())
            {
                TransmitPacketPtr possiblePacket;
                auto result = commands.front()->update(possiblePacket, this->g_server->getAddressType(), *client,
                                                       this->g_commandsUpdateTick);
                if (result == NetCommandResults::SUCCESS || result == NetCommandResults::FAILURE)
                {
                    std::cout << (result == NetCommandResults::SUCCESS ? "SUCCESS" : "FAILURE") << std::endl;
                    commands.pop_front();
                }

                if (possiblePacket)
                {
                    //Applying options
                    possiblePacket->applyOptions(*client);

                    //Sending the packet
                    client->pushPacket(std::move(possiblePacket));
                }
            }
        }

        ++it;
    }

    if (this->g_commandsUpdateTick >= FGE_NET_CMD_UPDATE_TICK_MS)
    {
        this->g_lastCommandUpdateTimePoint = now;
        this->g_commandsUpdateTick = std::chrono::milliseconds::zero();
    }
}
FluxProcessResults
ServerNetFluxUdp::process(ClientSharedPtr& refClient, ReceivedPacketPtr& packet, bool allowUnknownClient)
{
    this->processClients();

    refClient.reset();
    packet.reset();

    if (this->_g_remainingPackets == 0)
    {
        this->_g_remainingPackets = this->getPacketsSize();
        return FluxProcessResults::NOT_RETRIEVABLE;
    }

    //Popping the next packet
    packet = this->popNextPacket();
    if (!packet)
    {
        this->_g_remainingPackets = this->getPacketsSize();
        return FluxProcessResults::NOT_RETRIEVABLE;
    }
    --this->_g_remainingPackets;

    //Verify if the client is known
    auto* refClientData = this->_clients.getData(packet->getIdentity());

    if (refClientData == nullptr)
    {
        //Unknown client

        if (allowUnknownClient)
        {
            //Check if the packet is fragmented
            if (packet->isFragmented())
            { //We can't (disallow) process fragmented packets from unknown clients
                return FluxProcessResults::NOT_RETRIEVABLE;
            }

            //Check if the packet is a handshake
            if (packet->retrieveHeaderId().value() == NET_INTERNAL_ID_FGE_HANDSHAKE)
            {
                std::cout << "Handshake received" << std::endl;

                using namespace fge::net::rules;
                std::string handshakeString;
                auto const err = RValid(RSizeMustEqual<std::string>(sizeof(FGE_NET_HANDSHAKE_STRING) - 1,
                                                                    {packet->packet(), &handshakeString}))
                                         .end();
                if (err || !packet->endReached() || handshakeString != FGE_NET_HANDSHAKE_STRING)
                { //TODO: endReached() check should be done in .end() method
                    std::cout << "Handshake failed" << std::endl;
                    return FluxProcessResults::NOT_RETRIEVABLE;
                }

                std::cout << "Handshake accepted" << std::endl;
                //Handshake accepted, we create the client
                refClient = std::make_shared<Client>();
                refClient->getStatus().setNetworkStatus(ClientStatus::NetworkStatus::ACKNOWLEDGED);
                refClient->getStatus().setTimeout(FGE_NET_STATUS_DEFAULT_TIMEOUT);
                this->_clients.add(packet->getIdentity(), refClient);
                //TODO: callback

                //Add MTU command as this is required for the next state
                auto* clientData = this->_clients.getData(packet->getIdentity());
                auto command = std::make_unique<NetMTUCommand>(&clientData->_commands);
                clientData->_mtuFuture = command->get_future();
                //At this point commands should be empty
                clientData->_commands.push_front(std::move(command));

                //Respond to the handshake
                auto responsePacket = CreatePacket(NET_INTERNAL_ID_FGE_HANDSHAKE);
                responsePacket->doNotReorder().doNotFragment().doNotDiscard();
                responsePacket->packet() << FGE_NET_HANDSHAKE_STRING;
                refClient->pushPacket(std::move(responsePacket));
                this->g_server->notifyTransmission();

                this->_onClientAcknowledged.call(refClient, packet->getIdentity());

                return FluxProcessResults::INTERNALLY_HANDLED;
            }

            //We can't process packets further for unknown clients
            return FluxProcessResults::NOT_RETRIEVABLE;
        }

        this->g_server->repushPacket(std::move(packet));
        return FluxProcessResults::NOT_RETRIEVABLE;
    }
    refClient = refClientData->_client;

    //Check if the client is in an acknowledged state
    if (refClient->getStatus().getNetworkStatus() == ClientStatus::NetworkStatus::ACKNOWLEDGED)
    {
        //At this point, the client still need to do the MTU discovery
        switch (packet->retrieveHeaderId().value())
        {
        case NET_INTERNAL_ID_MTU_TEST:
        {
            auto response = CreatePacket(NET_INTERNAL_ID_MTU_TEST_RESPONSE);
            response->doNotDiscard().doNotReorder();
            refClient->pushPacket(std::move(response));
            refClient->getStatus().resetTimeout();
            std::cout << "received MTU test" << std::endl;
            break;
        }
        case NET_INTERNAL_ID_MTU_ASK:
        {
            auto response = CreatePacket(NET_INTERNAL_ID_MTU_ASK_RESPONSE);
            response->doNotDiscard().doNotReorder()
                    << SocketUdp::retrieveAdapterMTUForDestination(packet->getIdentity()._ip).value_or(0);
            refClient->pushPacket(std::move(response));
            refClient->getStatus().resetTimeout();
            std::cout << "received MTU ask" << std::endl;
            break;
        }
        case NET_INTERNAL_ID_MTU_FINAL:
            std::cout << "received MTU final" << std::endl;
            refClient->_mtuFinalizedFlag = true;
            refClient->getStatus().resetTimeout();
            //Client have finished the MTU discovery, but we have to check if the server have finished too
            if (refClient->getMTU() != 0)
            {
                if (!CryptServerCreate(static_cast<SSL_CTX*>(this->g_server->getCryptContext()), *refClient))
                {
                    std::cout << "CryptServerCreate failed" << std::endl;
                    //Discard the packet and the client
                    this->_clients.remove(packet->getIdentity());
                    this->_onClientDropped.call(refClient, packet->getIdentity());
                    return FluxProcessResults::NOT_RETRIEVABLE;
                }
                std::cout << "CryptServerCreate success, starting crypt exchange" << std::endl;
                refClient->getStatus().setNetworkStatus(ClientStatus::NetworkStatus::MTU_DISCOVERED);
                refClient->getStatus().setTimeout(FGE_NET_STATUS_DEFAULT_TIMEOUT);
                this->_onClientMTUDiscovered.call(refClient, packet->getIdentity());
            }
            break;
        default:
        {
            auto const result = this->checkCommands(refClient, refClientData->_commands, packet);
            if (result == NetCommandResults::FAILURE)
            {
                std::cout << "Command failed" << std::endl;
                //Discard the packet and the client
                this->_clients.remove(packet->getIdentity());
                this->_onClientDropped.call(refClient, packet->getIdentity());
                return FluxProcessResults::NOT_RETRIEVABLE;
            }

            if (result == NetCommandResults::SUCCESS)
            {
                std::cout << "Command success" << std::endl;
                refClient->setMTU(refClientData->_mtuFuture.get());

                auto response = CreatePacket(NET_INTERNAL_ID_MTU_FINAL);
                response->doNotDiscard().doNotReorder();
                refClient->pushPacket(std::move(response));
                refClient->getStatus().resetTimeout();

                //Server have finished the MTU discovery, but we have to check if the client have finished too
                if (refClient->_mtuFinalizedFlag)
                {
                    std::cout << "mtu finalized" << std::endl;
                    if (!CryptServerCreate(static_cast<SSL_CTX*>(this->g_server->getCryptContext()), *refClient))
                    {
                        std::cout << "CryptServerCreate failed" << std::endl;
                        //Discard the packet and the client
                        this->_clients.remove(packet->getIdentity());
                        this->_onClientDropped.call(refClient, packet->getIdentity());
                        return FluxProcessResults::NOT_RETRIEVABLE;
                    }

                    std::cout << "CryptServerCreate success, starting crypt exchange" << std::endl;
                    refClient->getStatus().setNetworkStatus(ClientStatus::NetworkStatus::MTU_DISCOVERED);
                    refClient->getStatus().setTimeout(FGE_NET_STATUS_DEFAULT_TIMEOUT);
                    this->_onClientMTUDiscovered.call(refClient, packet->getIdentity());
                }
            }
        }
        break;
        }
        //TODO: add a timeout for the MTU discovery
        return FluxProcessResults::INTERNALLY_HANDLED;
    }

    //Check if the packet is a fragment
    if (packet->isFragmented())
    {
        auto const identity = packet->getIdentity();
        auto const result = refClientData->_defragmentation.process(std::move(packet));
        if (result._result == PacketDefragmentation::Results::RETRIEVABLE)
        {
            packet = refClientData->_defragmentation.retrieve(result._id, identity);
        }
        else
        {
            return FluxProcessResults::NOT_RETRIEVABLE;
        }
    }

    //Check if the client is in an MTU discovered state
    if (refClient->getStatus().getNetworkStatus() == ClientStatus::NetworkStatus::MTU_DISCOVERED)
    {
        //Check if the packet is a crypt handshake
        if (packet->retrieveHeaderId().value() != NET_INTERNAL_ID_CRYPT_HANDSHAKE)
        {
            return FluxProcessResults::NOT_RETRIEVABLE;
        }

        std::cout << "receiving crypt handshake" << std::endl;
        auto& info = refClient->getCryptInfo();

        refClient->getStatus().resetTimeout();

        auto const readPos = packet->getReadPos();
        BIO_write(static_cast<BIO*>(info._rbio), packet->getData() + readPos, packet->getDataSize() - readPos);

        auto const result = SSL_do_handshake(static_cast<SSL*>(info._ssl));
        if (result <= 0)
        {
            auto const err = SSL_get_error(static_cast<SSL*>(info._ssl), result);

            if (err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE)
            {
                ERR_print_errors_fp(stderr);
                refClient->getStatus().setNetworkStatus(ClientStatus::NetworkStatus::DISCONNECTED);
                this->_clients.remove(packet->getIdentity());
                this->_onClientDropped.call(refClient, packet->getIdentity());
                return FluxProcessResults::NOT_RETRIEVABLE;
            }
        }

        std::cout << "check for transmit crypt" << std::endl;
        // Check if OpenSSL has produced encrypted handshake data
        auto const pendingSize = BIO_ctrl_pending(static_cast<BIO*>(info._wbio));
        if (pendingSize == 0)
        {
            std::cout << "NONE" << std::endl;
            return FluxProcessResults::NOT_RETRIEVABLE;
        }
        std::cout << "transmitting crypt" << std::endl;
        auto response = CreatePacket(NET_INTERNAL_ID_CRYPT_HANDSHAKE);
        auto const packetStartDataPosition = response->doNotDiscard().getDataSize();
        response->append(pendingSize);
        auto const finalSize =
                BIO_read(static_cast<BIO*>(info._wbio), response->getData() + packetStartDataPosition, pendingSize);
        if (finalSize <= 0 || static_cast<std::size_t>(finalSize) != pendingSize)
        {
            std::cout << "failed crypt" << std::endl;
            refClient->getStatus().setNetworkStatus(ClientStatus::NetworkStatus::DISCONNECTED);
            this->_clients.remove(packet->getIdentity());
            this->_onClientDropped.call(refClient, packet->getIdentity());
            return FluxProcessResults::NOT_RETRIEVABLE;
        }
        refClient->pushPacket(std::move(response));
        this->g_server->notifyTransmission();

        if (SSL_is_init_finished(static_cast<SSL*>(info._ssl)) == 1)
        {
            std::cout << "CONNECTED" << std::endl;
            refClient->getStatus().setNetworkStatus(ClientStatus::NetworkStatus::CONNECTED);
            refClient->getStatus().setTimeout(FGE_NET_STATUS_DEFAULT_CONNECTED_TIMEOUT);
            this->_onClientConnected.call(refClient, packet->getIdentity());
            return FluxProcessResults::INTERNALLY_HANDLED;
        }

        return FluxProcessResults::INTERNALLY_HANDLED;
    }

    std::cout << "processing packet" << std::endl;

    auto const headerFlags = packet->retrieveFlags().value();

    if ((headerFlags & FGE_NET_HEADER_DO_NOT_REORDER_FLAG) > 0)
    {
        return FluxProcessResults::RETRIEVABLE;
    }

    if ((headerFlags & FGE_NET_HEADER_LOCAL_REORDERED_FLAG) == 0)
    {
        auto reorderResult = this->processReorder(*refClient, packet, refClient->getClientPacketCounter(), true);
        if (reorderResult != FluxProcessResults::RETRIEVABLE)
        {
            return reorderResult;
        }
    }

    //Verify the realm
    if (!this->verifyRealm(refClient, packet))
    {
        return FluxProcessResults::BAD_REALM;
    }

    auto const stat =
            PacketReorderer::checkStat(packet, refClient->getClientPacketCounter(), refClient->getCurrentRealm());

    if ((headerFlags & FGE_NET_HEADER_DO_NOT_DISCARD_FLAG) == 0)
    {
        if (stat == PacketReorderer::Stats::OLD_COUNTER)
        {
            refClient->advanceLostPacketCount();
            --this->_g_remainingPackets;
            return FluxProcessResults::NOT_RETRIEVABLE;
        }
    }

    if (stat == PacketReorderer::Stats::WAITING_NEXT_REALM || stat == PacketReorderer::Stats::WAITING_NEXT_COUNTER)
    {
        refClient->advanceLostPacketCount(); //We are missing a packet
    }

    auto const countId = packet->retrieveCounter().value();
    refClient->setClientPacketCounter(countId);

    return FluxProcessResults::RETRIEVABLE;
}

bool ServerNetFluxUdp::verifyRealm(ClientSharedPtr const& refClient, ReceivedPacketPtr const& packet)
{
    auto const headerFlags = packet->retrieveFlags().value();
    auto const clientProvidedRealm = packet->retrieveRealm().value();

    if (clientProvidedRealm != refClient->getCurrentRealm() &&
        refClient->getLastRealmChangeElapsedTime() >= FGE_SERVER_MAX_TIME_DIFFERENCE_REALM)
    {
        this->_onClientBadRealm.call(refClient);
        return (headerFlags & FGE_NET_HEADER_DO_NOT_DISCARD_FLAG) > 0;
    }
    return true;
}
NetCommandResults
ServerNetFluxUdp::checkCommands(ClientSharedPtr const& refClient, CommandQueue& commands, ReceivedPacketPtr& packet)
{
    if (commands.empty())
    {
        return NetCommandResults::FAILURE;
    }

    auto const result = commands.front()->onReceive(packet, this->g_server->getAddressType(), *refClient);
    if (result == NetCommandResults::SUCCESS || result == NetCommandResults::FAILURE)
    {
        commands.pop_front();
    }
    return result;
}

//ServerUdp
ServerSideNetUdp::ServerSideNetUdp(IpAddress::Types type) :
        g_threadReception(nullptr),
        g_threadTransmission(nullptr),
        g_defaultFlux(*this),
        g_socket(type),
        g_running(false)
{}
ServerSideNetUdp::~ServerSideNetUdp()
{
    this->stop();
}

bool ServerSideNetUdp::start(Port bindPort, IpAddress const& bindIp, IpAddress::Types addressType)
{
    if (this->g_running)
    {
        return false;
    }

    this->g_socket.setAddressType(addressType);
    if (this->g_socket.bind(bindPort, bindIp) == Socket::Errors::ERR_NOERROR)
    {
        if (!CryptServerInit(reinterpret_cast<std::add_lvalue_reference_t<SSL_CTX*>>(this->g_crypt_ctx)))
        {
            this->g_socket.close();
            return false;
        }

        this->g_running = true;

        this->g_threadReception = std::make_unique<std::thread>(&ServerSideNetUdp::threadReception, this);
        this->g_threadTransmission = std::make_unique<std::thread>(&ServerSideNetUdp::threadTransmission, this);

        return true;
    }
    return false;
}
bool ServerSideNetUdp::start(IpAddress::Types addressType)
{
    if (this->g_running)
    {
        return false;
    }
    this->g_socket.setAddressType(addressType);
    if (this->g_socket.isValid())
    {
        if (!CryptServerInit(reinterpret_cast<std::add_lvalue_reference_t<SSL_CTX*>>(this->g_crypt_ctx)))
        {
            this->g_socket.close();
            return false;
        }

        this->g_running = true;

        this->g_threadReception = std::make_unique<std::thread>(&ServerSideNetUdp::threadReception, this);
        this->g_threadTransmission = std::make_unique<std::thread>(&ServerSideNetUdp::threadTransmission, this);

        return true;
    }
    return false;
}
void ServerSideNetUdp::stop()
{
    if (this->g_running)
    {
        this->g_running = false;

        this->g_threadReception->join();
        this->g_threadTransmission->join();

        this->g_threadReception = nullptr;
        this->g_threadTransmission = nullptr;

        this->g_socket.close();

        //Clear the flux
        std::scoped_lock const lock(this->g_mutexServer);
        for (auto& flux: this->g_fluxes)
        {
            flux->clearPackets();
        }
        this->g_defaultFlux.clearPackets();
        decltype(this->g_transmissionQueue)().swap(this->g_transmissionQueue);

        CryptUninit(reinterpret_cast<std::add_lvalue_reference_t<SSL_CTX*>>(this->g_crypt_ctx));
    }
}

ServerNetFluxUdp* ServerSideNetUdp::newFlux()
{
    std::scoped_lock const lock(this->g_mutexServer);

    this->g_fluxes.push_back(std::make_unique<ServerNetFluxUdp>(*this));
    return this->g_fluxes.back().get();
}
ServerNetFluxUdp* ServerSideNetUdp::getFlux(std::size_t index)
{
    std::scoped_lock const lock(this->g_mutexServer);

    if (index >= this->g_fluxes.size())
    {
        return nullptr;
    }
    return this->g_fluxes[index].get();
}
ServerNetFluxUdp* ServerSideNetUdp::getDefaultFlux()
{
    return &this->g_defaultFlux;
}
std::size_t ServerSideNetUdp::getFluxSize() const
{
    return this->g_fluxes.size();
}
IpAddress::Types ServerSideNetUdp::getAddressType() const
{
    return this->g_socket.getAddressType();
}
void ServerSideNetUdp::closeFlux(NetFluxUdp* flux)
{
    std::scoped_lock const lock(this->g_mutexServer);

    for (std::size_t i = 0; i < this->g_fluxes.size(); ++i)
    {
        if (this->g_fluxes[i].get() == flux)
        {
            this->g_fluxes.erase(this->g_fluxes.begin() + i);
            break;
        }
    }
}
void ServerSideNetUdp::closeAllFlux()
{
    std::scoped_lock const lock(this->g_mutexServer);
    this->g_fluxes.clear();
}

void ServerSideNetUdp::repushPacket(ReceivedPacketPtr&& packet)
{
    if (!packet->checkFluxLifetime(this->g_fluxes.size()))
    {
        this->g_defaultFlux.pushPacket(std::move(packet));
        return;
    }
    this->g_fluxes[packet->getFluxIndex()]->forcePushPacket(std::move(packet));
}

void ServerSideNetUdp::notifyTransmission()
{
    this->g_transmissionNotifier.notify_one();
}

bool ServerSideNetUdp::isRunning() const
{
    return this->g_running;
}

void ServerSideNetUdp::sendTo(TransmitPacketPtr& pck, Client const& client, Identity const& id)
{
    pck->applyOptions(client);
    pck->doNotReorder();

    {
        std::scoped_lock const lock(this->g_mutexServer);
        this->g_transmissionQueue.emplace(std::move(pck), id);
    }
    this->g_transmissionNotifier.notify_one();
}
void ServerSideNetUdp::sendTo(TransmitPacketPtr& pck, Identity const& id)
{
    pck->applyOptions();
    pck->doNotReorder();

    {
        std::scoped_lock const lock(this->g_mutexServer);
        this->g_transmissionQueue.emplace(std::move(pck), id);
    }
    this->g_transmissionNotifier.notify_one();
}

void* ServerSideNetUdp::getCryptContext() const
{
    return this->g_crypt_ctx;
}

void ServerSideNetUdp::threadReception()
{
    Identity idReceive;
    Packet pckReceive; //TODO
    std::size_t pushingIndex = 0;

    while (this->g_running)
    {
        if (this->g_socket.select(true, FGE_SERVER_PACKET_RECEPTION_TIMEOUT_MS) == Socket::Errors::ERR_NOERROR)
        {
            if (this->g_socket.receiveFrom(pckReceive, idReceive._ip, idReceive._port) == Socket::Errors::ERR_NOERROR)
            {
#ifdef FGE_ENABLE_SERVER_NETWORK_RANDOM_LOST
                if (fge::_random.range(0, 1000) <= 10)
                {
                    continue;
                }
#endif

                std::scoped_lock const lck(this->g_mutexServer);

                if (pckReceive.getDataSize() < ProtocolPacket::HeaderSize)
                { //Bad header, packet is dismissed
                    continue;
                }

                //Skip the header for reading
                pckReceive.skip(ProtocolPacket::HeaderSize);
                auto packet = std::make_unique<ProtocolPacket>(std::move(pckReceive), idReceive);
                packet->setTimestamp(Client::getTimestamp_ms());

                //Verify headerId
                auto const headerId = packet->retrieveFullHeaderId().value();
                if ((headerId & ~FGE_NET_HEADER_FLAGS_MASK) == FGE_NET_BAD_ID ||
                    (headerId & FGE_NET_HEADER_LOCAL_REORDERED_FLAG) > 0)
                { //Bad headerId, packet is dismissed
                    continue;
                }

                //Realm and countId is verified by the flux who have the corresponding client

                if (this->g_fluxes.empty())
                {
                    this->g_defaultFlux.pushPacket(std::move(packet));
                    continue;
                }

                //Try to push packet in a flux
                for (std::size_t i = 0; i < this->g_fluxes.size(); ++i)
                {
                    pushingIndex = packet->bumpFluxIndex(this->g_fluxes.size());
                    if (this->g_fluxes[pushingIndex]->pushPacket(std::move(packet)))
                    { //Packet is correctly pushed
                        break;
                    }
                }
                //If every flux is busy, the new packet is dismissed
            }
        }
    }
}
void ServerSideNetUdp::threadTransmission()
{
    std::unique_lock lckServer(this->g_mutexServer);

    while (this->g_running)
    {
        this->g_transmissionNotifier.wait_for(lckServer, std::chrono::milliseconds(10));

        //Checking fluxes
        for (std::size_t i = 0; i < this->g_fluxes.size() + 1; ++i)
        {
            ClientList* clients{nullptr};
            if (i == this->g_fluxes.size())
            { //Doing the default flux
                clients = &this->g_defaultFlux._clients;
            }
            else
            {
                clients = &this->g_fluxes[i]->_clients;
            }

            auto clientLock = clients->acquireLock();

            for (auto itClient = clients->begin(clientLock); itClient != clients->end(clientLock); ++itClient)
            {
                if (itClient->second._client->isPendingPacketsEmpty())
                {
                    continue;
                }

                if (itClient->second._client->getLastPacketLatency() < itClient->second._client->getSTOCLatency_ms())
                {
                    continue;
                }

                auto transmissionPacket = itClient->second._client->popPacket();

                //MTU check
                if (!transmissionPacket->isFragmented() &&
                    !transmissionPacket->checkFlags(FGE_NET_HEADER_DO_NOT_FRAGMENT_FLAG))
                {
                    auto const mtu = itClient->second._client->getMTU();

                    //Packet is not fragmented, we have to check is size
                    if (mtu == 0)
                    { //We don't know the MTU yet
                        goto mtu_check_skip;
                    }

                    auto fragments = transmissionPacket->fragment(mtu);
                    for (std::size_t iFragment = 0; iFragment < fragments.size(); ++iFragment)
                    {
                        if (iFragment == 0)
                        {
                            transmissionPacket = std::move(fragments[iFragment]);
                        }
                        else
                        {
                            itClient->second._client->pushForcedFrontPacket(std::move(fragments[iFragment]));
                        }
                    }
                }
            mtu_check_skip:

                if (!transmissionPacket->packet() || !transmissionPacket->haveCorrectHeaderSize())
                { //Last verification of the packet
                    continue;
                }

                //Applying options
                transmissionPacket->applyOptions(*itClient->second._client);

                //Check if the packet must be encrypted
                if (transmissionPacket->isMarkedForEncryption())
                {
                    auto& info = itClient->second._client->getCryptInfo();
                    SSL_write(static_cast<SSL*>(info._ssl), transmissionPacket->getData(), transmissionPacket->getDataSize());
                    std::cout << "SSL_write: " << transmissionPacket->getDataSize() << std::endl;
                    transmissionPacket->clear();
                    auto const pendingSize = BIO_ctrl_pending(static_cast<BIO*>(info._wbio));
                    std::cout << pendingSize << std::endl;
                    if (pendingSize == 0)
                    {
                        continue;
                    }
                    transmissionPacket->append(pendingSize);
                    BIO_read(static_cast<BIO*>(info._wbio), transmissionPacket->getData(),
                             pendingSize);
                }

                //Sending the packet
                //TPacket packet(transmissionPacket->packet()); TODO
                this->g_socket.sendTo(transmissionPacket->packet(), itClient->first._ip, itClient->first._port);
                itClient->second._client->resetLastPacketTimePoint();
            }
        }

        //Checking isolated transmission queue
        while (!this->g_transmissionQueue.empty())
        {
            auto data = std::move(this->g_transmissionQueue.front());
            this->g_transmissionQueue.pop();

            if (!data.first->packet() || !data.first->haveCorrectHeaderSize())
            { //Last verification of the packet
                continue;
            }

            //Sending the packet
            //TPacket packet(data.first->packet()); TODO
            this->g_socket.sendTo(data.first->packet(), data.second._ip, data.second._port);
        }
    }
}

//ServerClientSideUdp
ClientSideNetUdp::ClientSideNetUdp(IpAddress::Types addressType) :
        g_threadReception(nullptr),
        g_threadTransmission(nullptr),
        g_socket(addressType),
        g_running(false),
        g_crypt_ctx(nullptr)
{
    this->_client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::DISCONNECTED);
}

ClientSideNetUdp::~ClientSideNetUdp()
{
    this->stop();
}

bool ClientSideNetUdp::start(Port bindPort,
                             IpAddress const& bindIp,
                             Port connectRemotePort,
                             IpAddress const& connectRemoteAddress,
                             IpAddress::Types addressType)
{
    if (this->g_running)
    {
        return false;
    }

    this->g_socket.setAddressType(addressType);
    if (addressType == IpAddress::Types::Ipv6)
    {
        this->g_socket.setIpv6Only(false);
    }
    else
    {
        this->g_socket.setDontFragment(true);
    }

    this->_client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::DISCONNECTED);

    if (this->g_socket.bind(bindPort, bindIp) == Socket::Errors::ERR_NOERROR)
    {
        if (this->g_socket.connect(connectRemoteAddress, connectRemotePort) != Socket::Errors::ERR_NOERROR)
        {
            this->g_socket.close();
            return false;
        }

        if (!CryptClientInit(reinterpret_cast<std::add_lvalue_reference_t<SSL_CTX*>>(this->g_crypt_ctx)))
        {
            this->g_socket.close();
            return false;
        }
        if (!CryptClientCreate(static_cast<SSL_CTX*>(this->g_crypt_ctx), this->_client))
        {
            this->g_socket.close();
            return false;
        }

        this->g_clientIdentity._ip = connectRemoteAddress;
        this->g_clientIdentity._port = connectRemotePort;

        this->g_running = true;

        this->g_threadReception = std::make_unique<std::thread>(&ClientSideNetUdp::threadReception, this);
        this->g_threadTransmission = std::make_unique<std::thread>(&ClientSideNetUdp::threadTransmission, this);

        return true;
    }
    this->g_socket.close();
    return false;
}
void ClientSideNetUdp::stop()
{
    if (this->g_running)
    {
        this->g_running = false;

        this->g_threadReception->join();
        this->g_threadTransmission->join();

        this->g_threadReception = nullptr;
        this->g_threadTransmission = nullptr;

        this->g_socket.close();

        //Clear the flux
        this->clearPackets();
        //Clear client
        this->_client.clearPackets();
        this->_client.clearLostPacketCount();
        this->_client.setClientPacketCounter(0);
        this->_client.setCurrentRealm(FGE_NET_DEFAULT_REALM);
        this->_client.setCurrentPacketCounter(0);

        CryptClientDestroy(this->_client);
        CryptUninit(reinterpret_cast<std::add_lvalue_reference_t<SSL_CTX*>>(this->g_crypt_ctx));
    }
}

void ClientSideNetUdp::notifyTransmission()
{
    this->g_transmissionNotifier.notify_one();
}

IpAddress::Types ClientSideNetUdp::getAddressType() const
{
    return this->g_socket.getAddressType();
}

bool ClientSideNetUdp::isRunning() const
{
    return this->g_running;
}

std::future<uint16_t> ClientSideNetUdp::retrieveMTU()
{
    if (!this->g_running)
    {
        throw Exception("Cannot retrieve MTU without a running client");
    }

    auto command = std::make_unique<NetMTUCommand>(&this->g_commands);
    auto future = command->get_future();

    std::scoped_lock const lock(this->g_mutexCommands);
    this->g_commands.push_back(std::move(command));

    return future;
}
std::future<bool> ClientSideNetUdp::connect()
{
    if (!this->g_running)
    {
        throw Exception("Cannot connect without a running client");
    }

    auto command = std::make_unique<NetConnectCommand>(&this->g_commands);
    auto future = command->get_future();

    std::scoped_lock const lock(this->g_mutexCommands);
    this->g_commands.push_back(std::move(command));

    return future;
}

Identity const& ClientSideNetUdp::getClientIdentity() const
{
    return this->g_clientIdentity;
}

FluxProcessResults ClientSideNetUdp::process(ReceivedPacketPtr& packet)
{
    ///TODO: no lock ?
    packet.reset();

    auto const networkStatus = this->_client.getStatus().getNetworkStatus();
    if (networkStatus == ClientStatus::NetworkStatus::TIMEOUT ||
        networkStatus == ClientStatus::NetworkStatus::DISCONNECTED)
    {
        return FluxProcessResults::NOT_RETRIEVABLE;
    }

    if (this->_client.getStatus().isTimeout())
    {
        this->_client.getStatus().setNetworkStatus(ClientStatus::NetworkStatus::TIMEOUT);
        this->_onClientTimeout.call(*this);
        this->_g_remainingPackets = 0;
        this->clearPackets();
        return FluxProcessResults::NOT_RETRIEVABLE;
    }

    if (this->_g_remainingPackets == 0)
    {
        this->_g_remainingPackets = this->getPacketsSize();
        return FluxProcessResults::NOT_RETRIEVABLE;
    }

    //Popping the next packet
    packet = this->popNextPacket();
    if (!packet)
    {
        this->_g_remainingPackets = this->getPacketsSize();
        return FluxProcessResults::NOT_RETRIEVABLE;
    }
    --this->_g_remainingPackets;

    auto const headerFlags = packet->retrieveFlags().value();

    if ((headerFlags & FGE_NET_HEADER_DO_NOT_REORDER_FLAG) > 0)
    {
        return FluxProcessResults::RETRIEVABLE;
    }

    if ((headerFlags & FGE_NET_HEADER_LOCAL_REORDERED_FLAG) == 0)
    {
        auto reorderResult =
                this->processReorder(this->_client, packet, this->_client.getCurrentPacketCounter(), false);
        if (reorderResult != FluxProcessResults::RETRIEVABLE)
        {
            return reorderResult;
        }
    }

    auto const stat = PacketReorderer::checkStat(packet, this->_client.getCurrentPacketCounter(),
                                                 this->_client.getCurrentRealm());

    if ((headerFlags & FGE_NET_HEADER_DO_NOT_DISCARD_FLAG) == 0)
    {
        if (stat == PacketReorderer::Stats::OLD_REALM || stat == PacketReorderer::Stats::OLD_COUNTER)
        {
            this->_client.advanceLostPacketCount();
            --this->_g_remainingPackets;
            return FluxProcessResults::NOT_RETRIEVABLE;
        }
    }

    if (stat == PacketReorderer::Stats::WAITING_NEXT_REALM || stat == PacketReorderer::Stats::WAITING_NEXT_COUNTER)
    {
        this->_client.advanceLostPacketCount(); //We are missing a packet
    }

    auto const serverRealm = packet->retrieveRealm().value();
    auto const serverCountId = packet->retrieveCounter().value();

    this->_client.setCurrentPacketCounter(serverCountId);
    this->_client.setCurrentRealm(serverRealm);

    return FluxProcessResults::RETRIEVABLE;
}

std::size_t ClientSideNetUdp::waitForPackets(std::chrono::milliseconds time_ms)
{
    std::unique_lock lock(this->_g_mutexFlux);
    auto packetSize = this->_g_packets.size();
    if (packetSize > 0)
    {
        return packetSize;
    }

    this->g_receptionNotifier.wait_for(lock, time_ms);
    return this->_g_packets.size();
}

void ClientSideNetUdp::threadReception()
{
    Packet pckReceive; //TODO

    while (this->g_running)
    {
        if (this->g_socket.select(true, FGE_SERVER_PACKET_RECEPTION_TIMEOUT_MS) == Socket::Errors::ERR_NOERROR)
        {
            if (this->g_socket.receive(pckReceive) != Socket::Errors::ERR_NOERROR)
            {
                continue;
            }

#ifdef FGE_ENABLE_CLIENT_NETWORK_RANDOM_LOST
            if (fge::_random.range(0, 1000) <= 10)
            {
                continue;
            }
#endif

            //Decrypting the packet if needed
            auto const status = this->_client.getStatus().getNetworkStatus();
            if (status == ClientStatus::NetworkStatus::CONNECTED ||
                status == ClientStatus::NetworkStatus::AUTHENTICATED)
            {
                auto& info = this->_client.getCryptInfo();

                BIO_write(static_cast<BIO*>(info._rbio), pckReceive.getData(), pckReceive.getDataSize());

                auto const result = SSL_read(static_cast<SSL*>(info._ssl), pckReceive.getData(), pckReceive.getDataSize());
                if (result <= 0)
                {
                    continue;
                }
                pckReceive.shrink(pckReceive.getDataSize() - result);
            }

            if (pckReceive.getDataSize() < ProtocolPacket::HeaderSize)
            { //Bad header, packet is dismissed
                continue;
            }

            //Skip the header
            pckReceive.skip(ProtocolPacket::HeaderSize);
            auto packet = std::make_unique<ProtocolPacket>(std::move(pckReceive), this->g_clientIdentity);
            packet->setTimestamp(Client::getTimestamp_ms());

            //Verify headerId
            auto const headerId = packet->retrieveFullHeaderId().value();
            if ((headerId & ~FGE_NET_HEADER_FLAGS_MASK) == FGE_NET_BAD_ID ||
                (headerId & FGE_NET_HEADER_LOCAL_REORDERED_FLAG) > 0)
            { //Bad headerId, packet is dismissed
                continue;
            }

            //Check client status and reset timeout
            auto const networkStatus = this->_client.getStatus().getNetworkStatus();
            if (networkStatus != ClientStatus::NetworkStatus::TIMEOUT)
            {
                //TODO : check if we need to reset the timeout
                this->_client.getStatus().resetTimeout();
            }

            //MTU handling
            if (networkStatus == ClientStatus::NetworkStatus::ACKNOWLEDGED)
            {
                switch (headerId & ~FGE_NET_HEADER_FLAGS_MASK)
                {
                case NET_INTERNAL_ID_MTU_TEST:
                {
                    auto response = CreatePacket(NET_INTERNAL_ID_MTU_TEST_RESPONSE);
                    response->doNotDiscard().doNotReorder();
                    this->_client.pushPacket(std::move(response));
                    this->_client.getStatus().resetTimeout();
                    std::cout << "received MTU test" << std::endl;
                    continue;
                }
                case NET_INTERNAL_ID_MTU_ASK:
                {
                    auto response = CreatePacket(NET_INTERNAL_ID_MTU_ASK_RESPONSE);
                    response->doNotDiscard().doNotReorder() << this->g_socket.retrieveCurrentAdapterMTU().value_or(0);
                    this->_client.pushPacket(std::move(response));
                    this->_client.getStatus().resetTimeout();
                    std::cout << "received MTU ask" << std::endl;
                    continue;
                }
                case NET_INTERNAL_ID_MTU_FINAL:
                    std::cout << "received MTU final" << std::endl;
                    this->_client._mtuFinalizedFlag = true;
                    this->_client.getStatus().resetTimeout();
                    continue;
                }
            }

            //Check if the packet is a fragment
            if ((headerId & ~FGE_NET_HEADER_FLAGS_MASK) == NET_INTERNAL_FRAGMENTED_PACKET)
            {
                auto const result = this->g_defragmentation.process(std::move(packet));
                if (result._result == PacketDefragmentation::Results::RETRIEVABLE)
                {
                    packet = this->g_defragmentation.retrieve(result._id, this->g_clientIdentity);
                }
                else
                {
                    continue;
                }
            }

            //Checking commands
            {
                std::scoped_lock const commandLock(this->g_mutexCommands);
                if (!this->g_commands.empty())
                {

                    auto const result =
                            this->g_commands.front()->onReceive(packet, this->g_socket.getAddressType(), this->_client);
                    if (result == NetCommandResults::SUCCESS || result == NetCommandResults::FAILURE)
                    {
                        this->g_commands.pop_front();
                    }

                    //Commands can drop the packet
                    if (!packet)
                    {
                        continue;
                    }
                }
            }

            this->pushPacket(std::move(packet));
            this->g_receptionNotifier.notify_all();
        }
    }
}
void ClientSideNetUdp::threadTransmission()
{
    auto lastTimePoint = std::chrono::steady_clock::now();
    std::chrono::milliseconds commandsTime{0};

    std::unique_lock lckServer(this->_g_mutexFlux);

    while (this->g_running)
    {
        this->g_transmissionNotifier.wait_for(lckServer, std::chrono::milliseconds(10));

        auto const now = std::chrono::steady_clock::now();
        auto const deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTimePoint);
        lastTimePoint = now;

        //Checking commands
        commandsTime += deltaTime;
        if (commandsTime >= FGE_NET_CMD_UPDATE_TICK_MS)
        {
            commandsTime = std::chrono::milliseconds::zero();

            std::scoped_lock const commandLock(this->g_mutexCommands);
            if (!this->g_commands.empty())
            {
                TransmitPacketPtr possiblePacket;
                auto const result = this->g_commands.front()->update(possiblePacket, this->g_socket.getAddressType(),
                                                                     this->_client, commandsTime);
                if (result == NetCommandResults::SUCCESS || result == NetCommandResults::FAILURE)
                {
                    this->g_commands.pop_front();
                }

                if (possiblePacket)
                {
                    //Applying options
                    possiblePacket->applyOptions(this->_client);

                    //Sending the packet
                    this->_client.pushPacket(std::move(possiblePacket));
                }
            }
        }

        //Flux
        if (this->_client.isPendingPacketsEmpty())
        {
            continue;
        }

        if (this->_client.getLastPacketLatency() >= this->_client.getCTOSLatency_ms())
        { //Ready to send !
            auto transmissionPacket = this->_client.popPacket();

            //MTU check
            if (!transmissionPacket->isFragmented() &&
                !transmissionPacket->checkFlags(FGE_NET_HEADER_DO_NOT_FRAGMENT_FLAG))
            {
                //Packet is not fragmented, we have to check is size
                if (this->_client.getMTU() == 0)
                { //We don't know the MTU yet
                    goto mtu_check_skip;
                }

                auto fragments = transmissionPacket->fragment(this->_client.getMTU());
                for (std::size_t i = 0; i < fragments.size(); ++i)
                {
                    if (i == 0)
                    {
                        transmissionPacket = std::move(fragments[i]);
                    }
                    else
                    {
                        this->_client.pushForcedFrontPacket(std::move(fragments[i]));
                    }
                }
            }
        mtu_check_skip:

            if (!transmissionPacket->packet() || !transmissionPacket->haveCorrectHeaderSize())
            { //Last verification of the packet
                continue;
            }

            //Applying options
            transmissionPacket->applyOptions(this->_client);

            //Sending the packet
            //TPacket packet = transmissionPacket->packet(); TODO
            this->g_socket.send(transmissionPacket->packet());
            this->_client.resetLastPacketTimePoint();
        }
    }
}

} // namespace fge::net
