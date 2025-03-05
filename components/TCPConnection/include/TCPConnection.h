#pragma once

#include <stdint.h>
#include <unordered_map>

/* define 255 as start byte */
#define TCPCONNECTION_MESSAGE_FRAMESTART 255

/* define maximum supported bytes to be tranmitted at once */
#define TCPCONNECTION_MAX_SUPPORTED_TRANMISSION 255

/* if support for NetworkMessage is not enabled, use internal buffers instead */
#if defined(_ENABLE_NETWORKMESSAGE_SUPPORT)
    #include "NetworkMessage.h"

    /* define message as NetworkMessage */
    #define TCPConnMsgType NetworkMessage
#else
    #include <vector>

    /* define message as NetworkMessage */
    #define TCPConnMsgType std::vector<uint8_t>
#endif

class TCPConnection {
  public:
#if defined(_ENABLE_INTERNAL_ERR_SUPPORT)
    typedef enum _InternalErr {
        /* init fail errors */
        SocketInitFail = 1,
        SocketBindFail,

        /* server-specific socket errors */
        ServerSocketStartFail,
        ClientAcceptFail,

        /* client-specific socket error*/
        ClientSocketStartFail,

        /* socket-specific errors */
        SocketError,
        SocketClosed,
    /* specially designated errors -> for now, WIN support is not guaranteed */
    #if defined(_WINPLATFORM)
        WinsocketInitFail = 30,
    #endif
    } InternalErr_t;
#endif

    TCPConnection();
    ~TCPConnection();

    void initServerSocket(const int &port);
    /* @todo: implement support for client sockets */
    void initClientSocket(const int &port);
    void deinit(void);

    /* returns index to client connection */
    uint16_t acceptConnection(void);
    /* by default allows 65536 connections*/
    void     setClientConnectionsLimit(uint16_t p_clientLimit);

    void           write(uint16_t connectionId, TCPConnMsgType data);
    TCPConnMsgType read(uint16_t connectionId);

    /* @todo: implement callback functionality */
    void registerReadCallback();
    void registerWriteCallback();

#if defined(_ENABLE_INTERNAL_ERR_SUPPORT)
    uint32_t getInternalErrorReport(void);
    bool     checkIfErrorOccured(InternalErr_t err);
#endif

  private:
    uint16_t m_clientLimit;
    /* server connection socket */
    void *m_ConnectionSocket;

    /* vector containing clients connected */
    std::unordered_map<uint16_t, void *> m_InternalSocketConnections;

    void (*readCallback)();
    void (*writeCallback)();

#if defined(_ENABLE_INTERNAL_ERR_SUPPORT)
    uint32_t m_InternalErrorReport = 0;
#endif

    /* internal function to initialize connection */
    void initConnectionSocket(void);
    void terminateConnectionSocket(void);
    void terminateClientSockets(void);

    /* internal message receival and validation */
    bool validateMessage(int byteCounter);
};
