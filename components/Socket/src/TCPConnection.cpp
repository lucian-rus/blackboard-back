#include "TCPConnection.h"
#include "logger.h"

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
    : m_ServerEnabled(false), m_ClientEnabled(false), m_ConnectionSocket(nullptr), m_InternalSocket(nullptr), readCallback(nullptr), writeCallback(nullptr) {
    _LOG_MESSAGE(_LOG_INFO, "instance of TCPConnection created");
}

TCPConnection::~TCPConnection() {
    this->deinit();
    _LOG_MESSAGE(_LOG_INFO, "instance of TCPConnection destroyed");
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

void TCPConnection::initServer(const int &port) {
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

    // since this is a server, create and hold the remote socket as the r/w socket
    this->m_InternalSocket = new SOCKET;

    _LOG_MESSAGE(_LOG_INFO, "server init ok");
}

void TCPConnection::startServer(void) {
    _LOG_MESSAGE(_LOG_INFO, "server start called");

    if (nullptr == this->m_InternalSocket) {
        _LOG_MESSAGE(_LOG_WARNING, "server start failed");
#if defined(_ENABLE_INTERNAL_ERR_SUPPORT)
        m_InternalErrorReport = 1 << TCPConnection::ServerStartFail;
#endif

        /* return here as server creation failed */
        return;
    }
    sockaddr_in client;
#if defined(_LINUXPLATFORM)
    /* not using `uint32_t` for consistency */
    unsigned int clientSize = sizeof(client);
#endif

    (*static_cast<SOCKET *>(this->m_InternalSocket)) = accept(*static_cast<SOCKET *>(this->m_ConnectionSocket), reinterpret_cast<sockaddr *>(&client), &clientSize);

    /* since this is a server, hold the remote socket as the r/w socket */
    _LOG_MESSAGE(_LOG_INFO, "new connection accepted");
#if defined(_LINUXPLATFORM)
    close(*static_cast<SOCKET *>(this->m_ConnectionSocket));
#endif
    delete static_cast<SOCKET *>(this->m_ConnectionSocket);

    while (true == this->m_ServerEnabled) {
        /* @todo: this is temp, just for checking connection receival */
        uint8_t buff[255];

        int bytes = recv(*static_cast<SOCKET *>(this->m_InternalSocket), buff, 1, 0);
        if (SOCKET_ERR == bytes) {
            _LOG_MESSAGE(_LOG_WARNING, "socket error. closing socket");
#if defined(_ENABLE_INTERNAL_ERR_SUPPORT)
            m_InternalErrorReport = 1 << TCPConnection::ServerStartFail;
#endif
            /* jump to destruction*/
            break;
        }

        if (0 == bytes) {
            _LOG_MESSAGE(_LOG_WARNING, "socket connection closed");
#if defined(_ENABLE_INTERNAL_ERR_SUPPORT)
            m_InternalErrorReport = 1 << TCPConnection::ServerStartFail;
#endif

            /* jump to destruction*/
            break;
        }

        /* @todo: check TCPMessage, which can be bunch of bytes */
        if (1 == bytes) {
            /* @todo: read message function */
        }
    }

    this->~TCPConnection();
    _LOG_MESSAGE(_LOG_INFO, "server stop ok");
}

/* @todo: add protections for deinit, like checking the status of the server/client */
void TCPConnection::deinit(void) {
    _LOG_MESSAGE(_LOG_INFO, "deinit called");

#ifdef _LINUXPLATFORM
    close((*static_cast<SOCKET *>(this->m_InternalSocket)));
#endif

    delete (static_cast<SOCKET *>(this->m_InternalSocket));
}
