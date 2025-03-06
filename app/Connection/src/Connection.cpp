#include "Connection.h"
#include "Logger.h"

Connection::Connection(TCPConnection *p_socket) {
    /* set connection reference as internal socket reference */
    this->m_InternalSocketReference = p_socket;

    this->m_SocketId       = this->m_InternalSocketReference->acceptConnection();
    TCPConnMsgType message = this->m_InternalSocketReference->read(this->m_SocketId);

    /* update this to handle NetworkMessage type */
#if defined(_ENABLE_TX_APP)
    if (0xA1 == message[0] && 0xA1 == message[2] && 0xB1 == message[1] && 0xB1 == message[3]) {
        this->m_ConnectionType = ConnectionType_t::ConnectionType_Incoming;
        _LOG_MESSAGE(_LOG_INFO, "got an APK connection");
    }
#endif

#if defined(_ENABLE_RX_APP)
    if (0xA9 == message[0] && 0xA9 == message[2] && 0xB9 == message[1] && 0xB9 == message[3]) {
        this->m_ConnectionType = ConnectionType_t::ConnectionType_Outgoing;
        _LOG_MESSAGE(_LOG_INFO, "got an APP connection");
    }
#endif
}

Connection::~Connection() {
}

void Connection::executeAssociatedFunction(TCPConnMsgType *message) {
    if (ConnectionType_t::ConnectionType_Incoming == this->m_ConnectionType) {
        _LOG_MESSAGE(_LOG_INFO, "receiving message");
        (*message) = this->m_InternalSocketReference->read(this->m_SocketId);
    }

    if (ConnectionType_t::ConnectionType_Outgoing == this->m_ConnectionType) {
        _LOG_MESSAGE(_LOG_INFO, "sending message");
        this->m_InternalSocketReference->write(this->m_SocketId, (*message));
    }
}

Connection::ConnectionType_t Connection::getConnectionType(void) {
    return this->m_ConnectionType;
}
