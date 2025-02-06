#include <algorithm>
#include <cstring>
#include <iostream>
#include <memory>

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

// this server was developed in windows and for now, adapt linux stuff to windows
#endif

#ifdef _LINUXPLATFORM
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>

    // define socket error from windows
    #define SOCKET_ERROR -1

    // define invalid socket from windows
    #define INVALID_SOCKET -1

// define SOCKET type to use the same code
typedef int SOCKET;
#endif

#ifdef _LINUXPLATFORM
    #include <thread>
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
    socketAddress.sin_family = AF_INET;
    socketAddress.sin_port   = htons(port);

#ifdef _WINPLATFORM
    socketAddress.sin_addr.S_un.S_addr = INADDR_ANY;
#endif

#ifdef _LINUXPLATFORM
    socketAddress.sin_addr.s_addr = INADDR_ANY;
#endif

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
#ifdef _WINPLATFORM
    closesocket(*(SOCKET *)this->m_Socket);
    WSACleanup();
#endif

#ifdef _LINUXPLATFORM
    close(*(SOCKET *)this->m_ConnectionSocket);
#endif

    delete (SOCKET *)this->m_Socket;
    return VHALSOCKET_STATUS_OK;
}

int32_t VHALSocket::startServer(void) {
    _LOG_MESSAGE(_LOG_INFO, "server start called");

    if (this->m_ShouldServerRun == false) {
        _LOG_VALUE(_LOG_WARNING, "server start failed, status: ", VHALSOCKET_STATUS_SERVER_NOT_INIT);
        return VHALSOCKET_STATUS_SERVER_NOT_INIT;
    }

    sockaddr_in client;
#ifdef _WINPLATFORM
    int clientSize = sizeof(client);
#endif

#ifdef _LINUXPLATFORM
    // not using `uint32_t` for consistency
    unsigned int clientSize = sizeof(client);
#endif

    *(SOCKET *)this->m_Socket = accept(*(SOCKET *)this->m_ConnectionSocket, (sockaddr *)&client, &clientSize);
    // since this is a server, hold the remote socket as the r/w socket
    _LOG_MESSAGE(_LOG_INFO, "new connection accepted called");

    this->m_ConnectionEstablished = true;

    // C-style casts cause I can't do it with C++ casts
#ifdef _WINPLATFORM
    closesocket(*(SOCKET *)this->m_ConnectionSocket);
#endif

#ifdef _LINUXPLATFORM
    close(*(SOCKET *)this->m_ConnectionSocket);
#endif

    delete (SOCKET *)this->m_ConnectionSocket;

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

int32_t VHALSocket::registerReadCallback(void (*callbackFunction)(std::vector<uint8_t>)) {
    this->readCallback = callbackFunction;
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

    // @todo: remove this once i get a better way of sending data back and forth
    std::vector<uint8_t> fwdBuffer(buffer, buffer + sizeof(buffer)/sizeof(buffer[0]));

    for(size_t i = 0; i < fwdBuffer.size(); i++) {
        std::cout << (int)fwdBuffer[i] << ' ';
    }
    std::cout << "\n";
    this->readCallback(fwdBuffer);

    if (nullptr != this->readCallback) {
        _LOG_MESSAGE(_LOG_INFO, "calling receival callback");
    }

    this->m_Queue.push(std::vector<uint8_t>(&buffer[0], &buffer[0] + size));
    _LOG_BUFFER(_LOG_INFO, "received buffer: ", buffer, size);
    return VHALSOCKET_STATUS_OK;
}

static VHALSocket socketMaster;
static VHALSocket socketSlave;

// this now works, but is not the best way to do this
void callbackFn(const std::vector<uint8_t> buffer) {
    socketSlave.write(buffer);

    // if we get an exit signal, just kill
    if(255 == buffer[0]) {
        socketSlave.stop();
        socketMaster.stop();

        socketSlave.deinit();
        socketMaster.deinit();
    }
}

// @todo: start working on tcp gibberish protection
// @todo: start working on frame integrity
void startMasterServer(void) {
    socketMaster.initServer(16080);
    // @todo: add protections
    socketMaster.registerReadCallback(callbackFn);

    socketMaster.startServer();
}

void startSlaveServer(void) {
    socketSlave.initServer(16081);
    socketSlave.startServer();
}

int main(void) {

#ifdef _LINUXPLATFORM
    std::thread t1(startMasterServer);
    std::thread t2(startSlaveServer);

    t1.join();
    t2.join();
#endif

#ifdef _WINPLATFORM
    startMasterServer();
#endif

    return 0;
}