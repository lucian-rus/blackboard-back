#include "TCPConnection.h"
#include "logger.h"

#include <cstring>
#include <memory>

/** WIN support will be added in the future */
#if defined(_LINUXPLATFORM)
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>

    /* define generic data type */
    #define SOCKET_ERR -1

/* define SOCKET data type */
typedef int SOCKET;
#endif

/* define generic status OK type */
#define STATUS_OK 0

TCPConnection::TCPConnection() : m_ServerEnabled(false), m_ClientEnabled(false), m_ConnectionSocket(nullptr), readCallback(nullptr), writeCallback(nullptr) {
    this->initConnectionSocket();
    _LOG_MESSAGE(_LOG_INFO, "instance of TCPConnection created");
}

TCPConnection::~TCPConnection() {
    this->deinit();
    _LOG_MESSAGE(_LOG_INFO, "instance of TCPConnection destroyed");
}

void TCPConnection::initServerSocket(const int &port) {
    _LOG_MESSAGE(_LOG_INFO, "server init called");

    if (nullptr == this->m_ConnectionSocket) {
        _LOG_MESSAGE(_LOG_WARNING, "server init failed. socket was not created");
        return;
    }

    sockaddr_in socketAddress;
    socketAddress.sin_family = AF_INET;
    socketAddress.sin_port   = htons(port);

#if defined(_LINUXPLATFORM)
    socketAddress.sin_addr.s_addr = INADDR_ANY;
#endif

    /* requires a reinterpret cast here as this is dumb */
    if (STATUS_OK != bind((*static_cast<SOCKET *>(this->m_ConnectionSocket)), reinterpret_cast<sockaddr *>(&socketAddress), sizeof(socketAddress))) {
        _LOG_MESSAGE(_LOG_WARNING, "socket bind failed");
#if defined(_ENABLE_INTERNAL_ERR_SUPPORT)
        m_InternalErrorReport = 1 << TCPConnection::SocketBindFail;
#endif

        /* return here as bind operation failed */
        return;
    }
    if (STATUS_OK != listen((*static_cast<SOCKET *>(this->m_ConnectionSocket)), SOMAXCONN)) {
        _LOG_MESSAGE(_LOG_WARNING, "socket listener start failed");
#if defined(_ENABLE_INTERNAL_ERR_SUPPORT)
        m_InternalErrorReport = 1 << TCPConnection::SocketStartFail;
#endif
        /* return here as listener creation failed*/
        return;
    }

    _LOG_MESSAGE(_LOG_INFO, "server init ok");
}

uint16_t TCPConnection::acceptConnection(void) {
    _LOG_MESSAGE(_LOG_INFO, "server start called");

    // since this is a server, create and hold the remote socket as the r/w socket
    uint16_t socketIndex                           = this->m_InternalSocketConnections.size();
    this->m_InternalSocketConnections[socketIndex] = new SOCKET;

    sockaddr_in client;
#if defined(_LINUXPLATFORM)
    /* not using `uint32_t` for consistency */
    unsigned int clientSize = sizeof(client);
#endif

    (*static_cast<SOCKET *>(this->m_InternalSocketConnections[socketIndex]))
        = accept(*static_cast<SOCKET *>(this->m_ConnectionSocket), reinterpret_cast<sockaddr *>(&client), &clientSize);

    if (SOCKET_ERR == (*static_cast<SOCKET *>(this->m_InternalSocketConnections[socketIndex]))) {
        this->m_InternalSocketConnections.erase(socketIndex);
        _LOG_MESSAGE(_LOG_WARNING, "error acception new connection");

        /* @todo: treat error properly */
        return 0xFFFF;
    }

    _LOG_VALUE(_LOG_INFO, "new connection accepted, index ", socketIndex);
    return socketIndex;
}

/* @todo: add protections for deinit, like checking the status of the server/client */
void TCPConnection::deinit(void) {
    _LOG_MESSAGE(_LOG_INFO, "deinit called");

    /* add protections here */
#ifdef _LINUXPLATFORM
    close((*static_cast<SOCKET *>(this->m_ConnectionSocket)));
    _LOG_MESSAGE(_LOG_INFO, "closing connection");

    for (auto &connection : this->m_InternalSocketConnections) {
        close(*static_cast<SOCKET *>(connection.second));
        _LOG_VALUE(_LOG_INFO, "closing client socket, index ", connection.first);
    }
#endif

    delete (static_cast<SOCKET *>(this->m_ConnectionSocket));
    for (auto &connection : this->m_InternalSocketConnections) {
        _LOG_VALUE(_LOG_INFO, "deleting client socket, index ", connection.first);
        delete (static_cast<SOCKET *>(connection.second));
    }

    this->m_InternalSocketConnections.clear();
}

TCPConnMsgType TCPConnection::read(uint16_t connectionId) {
    /* using c-style arrays of max 255 bytes as this is the current support */
    uint8_t l_TemporaryBuffer[255]  = {0};
    uint8_t l_sizeOfTemporaryBuffer = 0;

    /* receive some bytes */
    int byteCounter = recv(*static_cast<SOCKET *>(this->m_InternalSocketConnections[connectionId]), l_TemporaryBuffer, 1, 0);

    if (true == validateMessage(byteCounter)) {
        byteCounter             = recv(*static_cast<SOCKET *>(this->m_InternalSocketConnections[connectionId]), l_TemporaryBuffer, 1, 0);
        l_sizeOfTemporaryBuffer = l_TemporaryBuffer[0];

        byteCounter = recv(*static_cast<SOCKET *>(this->m_InternalSocketConnections[connectionId]), l_TemporaryBuffer, l_sizeOfTemporaryBuffer, 0);
        _LOG_BUFFER(_LOG_INFO, "buffer received: ", l_TemporaryBuffer, byteCounter);
    }

#if defined(_ENABLE_TCPMESSAGE_SUPPORT)
    #error "not yet supported"
#else
    TCPConnMsgType message(l_TemporaryBuffer, l_TemporaryBuffer + l_sizeOfTemporaryBuffer);
#endif

    return message;
}

void TCPConnection::write(uint16_t connectionId, const TCPConnMsgType &data) {
    // check if this works as intended. ported from VHALSOCKET
    std::unique_ptr<uint8_t[]> aux = std::make_unique<uint8_t[]>(255);
    aux[0]                         = 255;
    aux[1]                         = data.size();

    std::copy(data.begin(), data.end(), aux.get() + 2);
    send(*static_cast<SOCKET *>(this->m_InternalSocketConnections[connectionId]), reinterpret_cast<char *>(aux.get()), data.size() + 2, 0);
    _LOG_BUFFER(_LOG_INFO, "buffer sent: ", data.data(), data.size());
}

void TCPConnection::initConnectionSocket(void) {
    _LOG_MESSAGE(_LOG_INFO, "init socket called");
    this->m_ConnectionSocket = new SOCKET;

    _LOG_MESSAGE(_LOG_INFO, "creating socket...");
    (*static_cast<SOCKET *>(this->m_ConnectionSocket)) = socket(AF_INET, SOCK_STREAM, 0);
    if (SOCKET_ERR == (*static_cast<SOCKET *>(this->m_ConnectionSocket))) {
        _LOG_MESSAGE(_LOG_WARNING, "socket creation failed");
#if defined(_ENABLE_INTERNAL_ERR_SUPPORT)
        m_InternalErrorReport = 1 << TCPConnection::ServerInitFail;
#endif

        /* return as there was no socket created */
        return;
    }

    _LOG_MESSAGE(_LOG_INFO, "socket init ok");
}

bool TCPConnection::validateMessage(int byteCounter) {
    if (1 == byteCounter) {
        _LOG_MESSAGE(_LOG_INFO, "frame received on socket");
        return true;
    }

    if (SOCKET_ERR == byteCounter) {
        _LOG_MESSAGE(_LOG_WARNING, "socket error. closing socket");
#if defined(_ENABLE_INTERNAL_ERR_SUPPORT)
        m_InternalErrorReport = 1 << TCPConnection::ServerStartFail;
#endif

        /* jump to destruction */
        this->m_ServerEnabled = false;
    }

    if (0 == byteCounter) {
        _LOG_MESSAGE(_LOG_WARNING, "socket connection closed");
#if defined(_ENABLE_INTERNAL_ERR_SUPPORT)
        m_InternalErrorReport = 1 << TCPConnection::ServerStartFail;
#endif

        /* jump to destruction */
        this->m_ServerEnabled = false;
    }

    return false;
}