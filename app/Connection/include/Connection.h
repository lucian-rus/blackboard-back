#pragma once

/* projects includes */
#include "TCPConnection.h"

/* generic includes */
#include <stdint.h>

// should update this to inheritance
class Connection {
  public:
    typedef enum _ConnectionType {
        ConnectionType_Incoming = 0,
        ConnectionType_Outgoing,
        /* also define none, just for safety */
        ConnectionType_None,
    } ConnectionType_t;

    Connection(TCPConnection *p_socket);
    ~Connection();

    void executeAssociatedFunction(TCPConnMsgType *message);

  private:
    TCPConnection   *m_InternalSocketReference;
    ConnectionType_t m_ConnectionType;
    uint16_t         m_SocketId;
};
