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

TCPConnection::TCPConnection()
    : m_clientLimit(0xFFFF), m_ConnectionSocket(nullptr), readCallback(nullptr), writeCallback(nullptr) {
    this->initConnectionSocket();
    /* this does not take into account the empty unordered map */
    /* @todo: add unordered map init */
    _LOG_MESSAGE(_LOG_INFO, "instance of TCPConnection created");
}

TCPConnection::~TCPConnection() {
    this->deinit();
    _LOG_MESSAGE(_LOG_INFO, "instance of TCPConnection destroyed");
}

void TCPConnection::initServerSocket(const int &p_port) {
    _LOG_MESSAGE(_LOG_INFO, "server init called");

    if (nullptr == this->m_ConnectionSocket) {
        _LOG_MESSAGE(_LOG_WARNING, "server init failed. socket was not created");
        return;
    }

    sockaddr_in socketAddress;
    socketAddress.sin_family = AF_INET;
    socketAddress.sin_port   = htons(p_port);

#if defined(_LINUXPLATFORM)
    socketAddress.sin_addr.s_addr = INADDR_ANY;
#endif

    /* requires a reinterpret cast here as this is dumb */
    if (STATUS_OK
        != bind((*static_cast<SOCKET *>(this->m_ConnectionSocket)), reinterpret_cast<sockaddr *>(&socketAddress),
                sizeof(socketAddress))) {
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
        m_InternalErrorReport = 1 << TCPConnection::ServerSocketStartFail;
#endif
        /* return here as listener creation failed*/
        return;
    }

    _LOG_MESSAGE(_LOG_INFO, "server socket init ok");
}

uint16_t TCPConnection::acceptConnection(void) {
    _LOG_MESSAGE(_LOG_INFO, "accept connection called");

    /* since this is a server, create and hold the remote socket as the r/w socket. index is based on the current size
     * of the unordered map */
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
        _LOG_MESSAGE(_LOG_WARNING, "error accepting new connection");

        /* @todo: treat error properly -> will probably update to try/catch */
#if defined(_ENABLE_INTERNAL_ERR_SUPPORT)
        m_InternalErrorReport = 1 << TCPConnection::ClientAcceptFail;
#endif
        return 0xFFFF;
    }
    _LOG_VALUE(_LOG_INFO, "new connection accepted, index ", socketIndex);

    if (m_clientLimit == this->m_InternalSocketConnections.size()) {
        _LOG_MESSAGE(_LOG_WARNING, "maximum number of allowed client connections reached");
        this->terminateConnectionSocket();
    }

    return socketIndex;
}
void TCPConnection::setClientConnectionsLimit(uint16_t p_clientLimit) {
    _LOG_VALUE(_LOG_INFO, "setting client connections limit: ", p_clientLimit);
    this->m_clientLimit = p_clientLimit;
}

/* @todo: add protections for deinit, like checking the status of the server/client */
void TCPConnection::deinit(void) {
    _LOG_MESSAGE(_LOG_INFO, "deinit called");

    /* add protections here */
    if (nullptr != this->m_ConnectionSocket) {
        this->terminateConnectionSocket();
    }

    this->terminateClientSockets();
}

TCPConnMsgType TCPConnection::read(uint16_t p_connectionId) {
    /* @todo: handle missing error handling */

    /* using c-style arrays of max 255 bytes as this is the current support */
    uint8_t l_temporaryBuffer[TCPCONNECTION_MAX_SUPPORTED_TRANMISSION] = {0};
    uint8_t l_sizeOfTemporaryBuffer                                    = 0;

    /* receive framestart byte */
    int l_byteCounter
        = recv(*static_cast<SOCKET *>(this->m_InternalSocketConnections[p_connectionId]), l_temporaryBuffer, 1, 0);
    _LOG_MESSAGE(_LOG_INFO, "message received. validating...");

    if (true == validateMessage(l_byteCounter)) {
        l_byteCounter
            = recv(*static_cast<SOCKET *>(this->m_InternalSocketConnections[p_connectionId]), l_temporaryBuffer, 1, 0);
        /* get size of the incoming buffer as read from the socket */
        l_sizeOfTemporaryBuffer = l_temporaryBuffer[0];

        l_byteCounter = recv(*static_cast<SOCKET *>(this->m_InternalSocketConnections[p_connectionId]),
                             l_temporaryBuffer, l_sizeOfTemporaryBuffer, 0);
        _LOG_BUFFER(_LOG_INFO, "buffer received: ", l_temporaryBuffer, l_byteCounter);
    }

#if defined(_ENABLE_NETWORKMESSAGE_SUPPORT)
    #error "not yet supported"
#else
    TCPConnMsgType message(l_temporaryBuffer, l_temporaryBuffer + l_sizeOfTemporaryBuffer);
#endif

    return message;
}

/* sends copy as this is a fast op. no need for move */
void TCPConnection::write(uint16_t p_connectionId, TCPConnMsgType p_data) {
    if (this->m_InternalSocketConnections.end() == this->m_InternalSocketConnections.find(p_connectionId)) {
        _LOG_MESSAGE(_LOG_WARNING, "socket connection does not exist");
#if defined(_ENABLE_INTERNAL_ERR_SUPPORT)
        m_InternalErrorReport = 1 << TCPConnection::SocketClosed;
#endif

        return;
    }

    /* add TCPCONNECTION_MESSAGE_FRAMESTART, identifier (if exists) and the size */
    uint8_t l_sizeOfTransmission = p_data.size() + 2;

    p_data.insert(p_data.begin(), l_sizeOfTransmission);
    p_data.insert(p_data.begin(), TCPCONNECTION_MESSAGE_FRAMESTART);
    send(*static_cast<SOCKET *>(this->m_InternalSocketConnections[p_connectionId]), &p_data[0], l_sizeOfTransmission,
         0);

    _LOG_BUFFER(_LOG_INFO, "buffer sent: ", p_data.data(), p_data.size());
}

void TCPConnection::initConnectionSocket(void) {
    _LOG_MESSAGE(_LOG_INFO, "init socket called");
    this->m_ConnectionSocket = new SOCKET;

    _LOG_MESSAGE(_LOG_INFO, "creating socket...");
    (*static_cast<SOCKET *>(this->m_ConnectionSocket)) = socket(AF_INET, SOCK_STREAM, 0);
    if (SOCKET_ERR == (*static_cast<SOCKET *>(this->m_ConnectionSocket))) {
        _LOG_MESSAGE(_LOG_WARNING, "socket creation failed");
#if defined(_ENABLE_INTERNAL_ERR_SUPPORT)
        m_InternalErrorReport = 1 << TCPConnection::SocketInitFail;
#endif

        /* return as there was no socket created */
        return;
    }

    _LOG_MESSAGE(_LOG_INFO, "socket init ok");
}

void TCPConnection::terminateConnectionSocket(void) {
    _LOG_MESSAGE(_LOG_INFO, "called terminate connection sockets");

#ifdef _LINUXPLATFORM
    close((*static_cast<SOCKET *>(this->m_ConnectionSocket)));
    _LOG_MESSAGE(_LOG_INFO, "closing connection");
#endif

    delete (static_cast<SOCKET *>(this->m_ConnectionSocket));
    this->m_ConnectionSocket = nullptr;
}

void TCPConnection::terminateClientSockets(void) {
    _LOG_MESSAGE(_LOG_INFO, "called terminate client sockets");

#ifdef _LINUXPLATFORM
    for (auto &connection : this->m_InternalSocketConnections) {
        close(*static_cast<SOCKET *>(connection.second));
        _LOG_VALUE(_LOG_INFO, "closing client socket, index ", connection.first);
    }
#endif

    for (auto &connection : this->m_InternalSocketConnections) {
        _LOG_VALUE(_LOG_INFO, "deleting client socket, index ", connection.first);
        delete (static_cast<SOCKET *>(connection.second));
    }

    this->m_InternalSocketConnections.clear();
}

bool TCPConnection::validateMessage(int p_byteCounter) {
    if (1 == p_byteCounter) {
        _LOG_MESSAGE(_LOG_INFO, "frame received on socket");
        return true;
    }

    if (SOCKET_ERR == p_byteCounter) {
        _LOG_MESSAGE(_LOG_WARNING, "socket error. closing socket");
#if defined(_ENABLE_INTERNAL_ERR_SUPPORT)
        m_InternalErrorReport = 1 << TCPConnection::SocketError;
#endif
    }

    if (0 == p_byteCounter) {
        _LOG_MESSAGE(_LOG_WARNING, "socket connection closed");
#if defined(_ENABLE_INTERNAL_ERR_SUPPORT)
        m_InternalErrorReport = 1 << TCPConnection::SocketClosed;
#endif
    }

    return false;
}