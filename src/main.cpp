#include <algorithm>
#include <iostream>
#include <memory>

#define _WINPLATFORM

/**
 * @todo:
 ***        implement transmission lock
 ***        implement read check for null queue (measure of safety for no callbacks)
 **         test this on linux
 ******     make this work with gnu -> mostly works
 **         check if logs are all good
 ***        check for memory leaks
 *****      update handling for bad cases
 */

#ifdef _WINPLATFORM
    #include <WS2tcpip.h>
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
#endif

#include "logger.h"
#include "server.h"

VHALSocket::VHALSocket()
    : m_ShouldServerRun(false), m_ShouldClientRun(false), m_ConnectionEstablished(false), m_ConnectionSocket(nullptr),
      m_Socket(nullptr), readCallback(nullptr), writeCallback(nullptr) {
    _LOG_MESSAGE(_LOG_INFO, "instance of VHALSocket created");
}

VHALSocket::~VHALSocket() {
    this->deinit();
    _LOG_MESSAGE(_LOG_INFO, "instance of VHALSocket destroyed");
}

int32_t VHALSocket::initServer(const int &port) {
    _LOG_MESSAGE(_LOG_INFO, "server init called");

    // @todo: make this portable to linux, return different types of errors
    if (VHALSOCKET_STATUS_OK != this->initSocket()) {
        _LOG_VALUE(_LOG_WARNING, "server init failed, status code: ", VHALSOCKET_STATUS_SOCKET_BIND_FAIL);
        return VHALSOCKET_STATUS_SERVER_INIT_FAIL;
    }

    sockaddr_in socketAddress;
    socketAddress.sin_family           = AF_INET;
    socketAddress.sin_port             = htons(port);
    socketAddress.sin_addr.S_un.S_addr = INADDR_ANY;

    // C-style casts cause I can't do it with C++ casts
    if (bind(*(SOCKET *)this->m_ConnectionSocket, (sockaddr *)&socketAddress, sizeof(socketAddress)) != 0) {
        _LOG_VALUE(_LOG_WARNING, "socket bind failed, status code: ", VHALSOCKET_STATUS_SOCKET_BIND_FAIL);
        return VHALSOCKET_STATUS_SOCKET_BIND_FAIL;
    }
    if (listen(*(SOCKET *)this->m_ConnectionSocket, SOMAXCONN) != 0) {
        _LOG_VALUE(_LOG_WARNING, "socket listener failed, status code: ", VHALSOCKET_STATUS_SOCKET_START_FAIL);
        return VHALSOCKET_STATUS_SOCKET_START_FAIL;
    }

    // since this is a server, create and hold the remote socket as the r/w socket
    this->m_Socket          = new SOCKET;
    this->m_ShouldServerRun = true;

    _LOG_VALUE(_LOG_INFO, "server init ok, status code: ", VHALSOCKET_STATUS_OK);
    return VHALSOCKET_STATUS_OK;
}

int32_t VHALSocket::deinit(void) {
    _LOG_MESSAGE(_LOG_INFO, "deinit called");
    closesocket(*(SOCKET *)this->m_Socket);
    delete (SOCKET *)this->m_Socket;

    WSACleanup();

    return VHALSOCKET_STATUS_OK;
}

int32_t VHALSocket::startServer(void) {
    _LOG_MESSAGE(_LOG_INFO, "server start called");

    if (this->m_ShouldServerRun == false) {
        _LOG_VALUE(_LOG_WARNING, "server start failed, status: ", VHALSOCKET_STATUS_SERVER_NOT_INIT);
        return VHALSOCKET_STATUS_SERVER_NOT_INIT;
    }

    sockaddr_in client;
    int         clientSize = sizeof(client);

    *(SOCKET *)this->m_Socket = accept(*(SOCKET *)this->m_ConnectionSocket, (sockaddr *)&client, &clientSize);
    // since this is a server, hold the remote socket as the r/w socket
    _LOG_MESSAGE(_LOG_INFO, "new connection accepted called");

    this->m_ConnectionEstablished = true;

    // this is not working with GCC on windows ???
    // uses memory for no reason if not in debug mode, so only compile it in debug mode
    // #ifdef _LOG_ENABLED
    //     char host[NI_MAXHOST];
    //     char service[NI_MAXHOST];
    //     memset(host, 0, sizeof(host));
    //     memset(service, 0, sizeof(service));

    //     if (getnameinfo((sockaddr *)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
    //         _LOG_STRING(_LOG_INFO, "host: ", host);
    //         _LOG_STRING(_LOG_INFO, "service: ", service);
    //     }
    //     else {
    //         inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
    //         _LOG_STRING(_LOG_INFO, "host: ", host);
    //         _LOG_VALUE(_LOG_INFO, "port: ", ntohs(client.sin_port));
    //     }
    // #endif

    // C-style casts cause I can't do it with C++ casts
    closesocket(*(SOCKET *)this->m_ConnectionSocket);
    delete (SOCKET *)this->m_ConnectionSocket;

    uint8_t test_buffer[253];
    // update this
    while (this->m_ShouldServerRun == true) {
        std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(VHALSOCKET_MAX_CHARACTER_COUNTER);

        int bytes = recv(*(SOCKET *)this->m_Socket, reinterpret_cast<char *>(buffer.get()), 1, 0);
        if (bytes == SOCKET_ERROR) {
            this->~VHALSocket();

            _LOG_MESSAGE(_LOG_WARNING, "socket error. closing socket");
            return VHALSOCKET_STATUS_SOCKET_ERROR;
        }

        if (bytes == 0) {
            this->~VHALSocket();

            _LOG_MESSAGE(_LOG_WARNING, "socket connection closed");
            return VHALSOCKET_STATUS_SOCKET_CLOSED;
        }

        if (bytes == 1) {
            this->readSocketBuffer(buffer.get());
        }
    }

    this->~VHALSocket();
    _LOG_MESSAGE(_LOG_INFO, "server stop ok");
    return VHALSOCKET_STATUS_OK;
}

int32_t VHALSocket::stop(void) {
    this->m_ShouldServerRun = false;
    this->m_ShouldClientRun = false;

    return VHALSOCKET_STATUS_OK;
}

int32_t VHALSocket::write(const std::vector<uint8_t> &buffer) {
    _LOG_MESSAGE(_LOG_INFO, "write buffer called");
    if (false == this->m_ConnectionEstablished) {
        return -1;
    }

    std::unique_ptr<uint8_t[]> aux = std::make_unique<uint8_t[]>(VHALSOCKET_MAX_CHARACTER_COUNTER + 2);
    aux[0]                         = VHALSOCKET_BUFFER_START_CHAR;
    aux[1]                         = buffer.size();

    std::copy(buffer.begin(), buffer.end(), aux.get() + VHALSOCKET_BUFFER_START_PADDING);
    send(*(SOCKET *)this->m_Socket, reinterpret_cast<char *>(aux.get()), buffer.size() + VHALSOCKET_BUFFER_START_PADDING, 0);
    _LOG_BUFFER(_LOG_INFO, "buffer sent: ", buffer.data(), buffer.size());

    return VHALSOCKET_STATUS_OK;
}

int32_t VHALSocket::read(std::vector<uint8_t> &buffer) {
    _LOG_MESSAGE(_LOG_INFO, "read buffer called");
    if (false == this->m_ConnectionEstablished) {
        return -1;
    }

    if (this->m_Queue.empty() == true) {
        return -1;
    }

    std::copy(this->m_Queue.front().begin(), this->m_Queue.front().end(), std::back_inserter(buffer));
    this->m_Queue.pop();
    _LOG_BUFFER(_LOG_INFO, "buffer read: ", buffer.data(), buffer.size());

    return VHALSOCKET_STATUS_OK;
}

int32_t VHALSocket::registerReadCallback(void (*callbackFunction)()) {
    readCallback = callbackFunction;
    return VHALSOCKET_STATUS_OK;
}

int32_t VHALSocket::registerWriteCallback(void (*callbackFunction)()) {
    writeCallback = callbackFunction;
    return VHALSOCKET_STATUS_OK;
}

int32_t VHALSocket::initSocket(void) {
    _LOG_MESSAGE(_LOG_INFO, "init socket called");

#ifdef _WINPLATFORM
    WSADATA wsData;
    WORD    ver       = MAKEWORD(2, 2);
    int     wsHandler = WSAStartup(ver, &wsData);

    if (wsHandler != 0) {
        _LOG_VALUE(_LOG_WARNING, "winsocket init failed, status code: ", VHALSOCKET_STATUS_WINSOCKET_INIT_FAIL);
        return VHALSOCKET_STATUS_WINSOCKET_INIT_FAIL;
    }
#endif

    _LOG_MESSAGE(_LOG_INFO, "creating socket");
    this->m_ConnectionSocket = new SOCKET;
    // C-style casts cause I can't do it with C++ casts
    *(SOCKET *)this->m_ConnectionSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (*(SOCKET *)this->m_ConnectionSocket == INVALID_SOCKET) {
        _LOG_VALUE(_LOG_WARNING, "socket creation failed, status code: ", VHALSOCKET_STATUS_SOCKET_INIT_FAIL);
        return VHALSOCKET_STATUS_SOCKET_INIT_FAIL;
    }

    _LOG_VALUE(_LOG_INFO, "socket init ok, status code: ", VHALSOCKET_STATUS_OK);
    return VHALSOCKET_STATUS_OK;
}

int32_t VHALSocket::readSocketBuffer(uint8_t *buffer) {
    if (VHALSOCKET_BUFFER_START_CHAR != buffer[0]) {
        return -1;
    }

    _LOG_MESSAGE(_LOG_INFO, "socket message received");
    memset(buffer, 0, VHALSOCKET_MAX_CHARACTER_COUNTER);
    recv(*(SOCKET *)this->m_Socket, reinterpret_cast<char *>(buffer), 1, 0);

    int size = static_cast<int>(buffer[0]);
    recv(*(SOCKET *)this->m_Socket, reinterpret_cast<char *>(buffer), size, 0);

    if (nullptr != readCallback) {
        _LOG_MESSAGE(_LOG_INFO, "calling receival callback");
        readCallback();
    }

    this->m_Queue.push(std::vector<uint8_t>(&buffer[0], &buffer[0] + size));
    _LOG_BUFFER(_LOG_INFO, "received buffer: ", buffer, size);
    return VHALSOCKET_STATUS_OK;
}

int main(void) {
    VHALSocket socket;
    socket.initServer(8080);
    socket.startServer();

    return 0;
}