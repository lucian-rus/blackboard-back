#pragma once

/* if support for TCPMessage is not enabled, use internal buffers instead */
#if defined(_ENABLE_TCPMESSAGE_SUPPORT)
    #include "TCPMessage.h"

    /* define message as TCPMessage */
    #define TCPConnMsgType TCPMessage
#else
    #include <stdint.h>
    #include <vector>

    /* define message as TCPMessage */
    #define TCPConnMsgType std::vector<uint8_t>
#endif

/* if support for Internal Error handling is enabled*/
#if defined(_ENABLE_INTERNAL_ERR_SUPPORT)
    #include <stdint.h>
#endif

class TCPConnection {
  public:
#if defined(_ENABLE_INTERNAL_ERR_SUPPORT)
    typedef enum _InternalErr {
        /* init fail errors */
        SocketInitFail = 1,
        ServerInitFail,
        SocketBindFail,
        SocketStartFail,
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

    void initServer(const int &port);
    void initClient(const int &port);
    void stop(void);

    void write(const TCPConnMsgType &data);
    void read(TCPConnMsgType &data);

    void registerReadCallback();
    void registerWriteCallback();

#if defined(_ENABLE_INTERNAL_ERR_SUPPORT)
    uint32_t getInternalErrorReport(void);
    bool     checkIfErrorOccured(InternalErr_t err);
#endif

  private:
    void* m_ConnectionSocket;

#if defined(_ENABLE_INTERNAL_ERR_SUPPORT)
    uint32_t m_InternalErrorReport = 0;
#endif

    void initConnectionSocket(void);

};