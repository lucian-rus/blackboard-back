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

#ifdef _LINUXPLATFORM
    socketAddress.sin_addr.s_addr = INADDR_ANY;
#endif

    if (STATUS_OK
        != bind(*static_cast<SOCKET *>(this->m_ConnectionSocket), reinterpret_cast<sockaddr *>(&socketAddress),
                sizeof(socketAddress))) {
        _LOG_VALUE(_LOG_WARNING, "socket bind failed, status code: ", VHALSOCKET_STATUS_SOCKET_BIND_FAIL);
        return VHALSOCKET_STATUS_SOCKET_BIND_FAIL;
    }
    if (STATUS_OK != listen(*static_cast<SOCKET *>(this->m_ConnectionSocket), SOMAXCONN)) {
        _LOG_VALUE(_LOG_WARNING, "socket listener failed, status code: ", VHALSOCKET_STATUS_SOCKET_START_FAIL);
        return VHALSOCKET_STATUS_SOCKET_START_FAIL;
    }

    // since this is a server, create and hold the remote socket as the r/w socket
    this->m_Socket          = new SOCKET;
    this->m_ShouldServerRun = true;

    _LOG_VALUE(_LOG_INFO, "server init ok, status code: ", VHALSOCKET_STATUS_OK);
    return VHALSOCKET_STATUS_OK;
}
