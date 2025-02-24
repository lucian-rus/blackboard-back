#include <cstring>
#include <memory>

#include "logger.h"
#include "server.h"

/** these macros are here just for readability sake */
#define STATUS_OK   0
#define STATUS_FAIL 1

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

// handle error on socket closure ???
int32_t VHALSocket::deinit(void) {
    _LOG_MESSAGE(_LOG_INFO, "deinit called");
#ifdef _WINPLATFORM
    closesocket(*static_cast<SOCKET *>(this->m_Socket));
    WSACleanup();
#endif

#ifdef _LINUXPLATFORM
    close(*static_cast<SOCKET *>(this->m_ConnectionSocket));
#endif

    delete static_cast<SOCKET *>(this->m_Socket);
    return VHALSOCKET_STATUS_OK;
}

int32_t VHALSocket::startServer(void) {
    _LOG_MESSAGE(_LOG_INFO, "server start called");

    if (false == this->m_ShouldServerRun) {
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

    *static_cast<SOCKET *>(this->m_Socket)
        = accept(*static_cast<SOCKET *>(this->m_ConnectionSocket), reinterpret_cast<sockaddr *>(&client), &clientSize);
    // since this is a server, hold the remote socket as the r/w socket
    _LOG_MESSAGE(_LOG_INFO, "new connection accepted called");

    this->m_ConnectionEstablished = true;

    // C-style casts cause I can't do it with C++ casts
#ifdef _WINPLATFORM
    closesocket(*static_cast<SOCKET *>(this->m_ConnectionSocket));
#endif

#ifdef _LINUXPLATFORM
    close(*static_cast<SOCKET *>(this->m_ConnectionSocket));
#endif

    delete static_cast<SOCKET *>(this->m_ConnectionSocket);

    // update this
    while (true == this->m_ShouldServerRun) {
        std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(VHALSOCKET_MAX_CHARACTER_COUNTER);

        // update this ?
        int bytes = recv(*static_cast<SOCKET *>(this->m_Socket), reinterpret_cast<char *>(buffer.get()), 1, 0);
        if (SOCKET_ERROR == bytes) {
            this->~VHALSocket();

            _LOG_MESSAGE(_LOG_WARNING, "socket error. closing socket");
            return VHALSOCKET_STATUS_SOCKET_ERROR;
        }

        if (0 == bytes) {
            this->~VHALSocket();

            _LOG_MESSAGE(_LOG_WARNING, "socket connection closed");
            return VHALSOCKET_STATUS_SOCKET_CLOSED;
        }

        if (1 == bytes) {
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
        return STATUS_FAIL;
    }

    // check if this works as intended
    std::unique_ptr<uint8_t[]> aux = std::make_unique<uint8_t[]>(VHALSOCKET_MAX_CHARACTER_COUNTER + 2);
    aux[0]                         = VHALSOCKET_BUFFER_START_CHAR;
    aux[1]                         = buffer.size();

    std::copy(buffer.begin(), buffer.end(), aux.get() + VHALSOCKET_BUFFER_START_PADDING);
    send(*static_cast<SOCKET *>(this->m_Socket), reinterpret_cast<char *>(aux.get()),
         buffer.size() + VHALSOCKET_BUFFER_START_PADDING, 0);
    _LOG_BUFFER(_LOG_INFO, "buffer sent: ", buffer.data(), buffer.size());

    return VHALSOCKET_STATUS_OK;
}

//@todo proper error handling
int32_t VHALSocket::read(std::vector<uint8_t> &buffer) {
    _LOG_MESSAGE(_LOG_INFO, "read buffer called");
    if (false == this->m_ConnectionEstablished) {
        return STATUS_FAIL;
    }

    if (true == this->m_Queue.empty()) {
        return STATUS_FAIL;
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

    if (STATUS_OK != wsHandler) {
        _LOG_VALUE(_LOG_WARNING, "winsocket init failed, status code: ", VHALSOCKET_STATUS_WINSOCKET_INIT_FAIL);
        return VHALSOCKET_STATUS_WINSOCKET_INIT_FAIL;
    }
#endif

    _LOG_MESSAGE(_LOG_INFO, "creating socket");
    this->m_ConnectionSocket = new SOCKET;

    *static_cast<SOCKET *>(this->m_ConnectionSocket) = socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == (*static_cast<SOCKET *>(this->m_ConnectionSocket))) {
        _LOG_VALUE(_LOG_WARNING, "socket creation failed, status code: ", VHALSOCKET_STATUS_SOCKET_INIT_FAIL);
        return VHALSOCKET_STATUS_SOCKET_INIT_FAIL;
    }

    _LOG_VALUE(_LOG_INFO, "socket init ok, status code: ", VHALSOCKET_STATUS_OK);
    return VHALSOCKET_STATUS_OK;
}

int32_t VHALSocket::readSocketBuffer(uint8_t *buffer) {
    if (VHALSOCKET_BUFFER_START_CHAR != buffer[0]) {
        return STATUS_FAIL;
    }

    _LOG_MESSAGE(_LOG_INFO, "socket message received");
    memset(buffer, 0, VHALSOCKET_MAX_CHARACTER_COUNTER);
    recv(*static_cast<SOCKET *>(this->m_Socket), reinterpret_cast<char *>(buffer), 1, 0);

    int size = static_cast<int>(buffer[0]);
    recv(*static_cast<SOCKET *>(this->m_Socket), reinterpret_cast<char *>(buffer), size, 0);

    // @todo: remove this once i get a better way of sending data back and forth
    std::vector<uint8_t> fwdBuffer(buffer, buffer + sizeof(buffer) / sizeof(buffer[0]));
    this->readCallback(fwdBuffer);

    if (nullptr != this->readCallback) {
        _LOG_MESSAGE(_LOG_INFO, "calling receival callback");
    }

    this->m_Queue.push(std::vector<uint8_t>(&buffer[0], &buffer[0] + size));
    _LOG_BUFFER(_LOG_INFO, "received buffer: ", buffer, size);
    return VHALSOCKET_STATUS_OK;
}
