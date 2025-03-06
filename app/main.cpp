#include <algorithm>

#include "Connection.h"

/**
// @todo: implement transmission lock
// @todo: update handling for bad cases
// @todo: update socket to handle different identification buffers
// @todo: maybe add crc just for fun
// @todo: start working on tcp gibberish protection
// @todo: start working on frame integrity
*/

static TCPConnection socketMaster;

void startMasterServer(void) {
    socketMaster.initServerSocket(_SOCKET_NUMBER);
    std::vector<Connection> connections;

#if defined(_CLIENT_CONN_LIMIT)
    socketMaster.setClientConnectionsLimit(_CLIENT_CONN_LIMIT);
#endif

    bool l_isVectorSorted = false;

    while (true) {
        if (_CLIENT_CONN_LIMIT != connections.size()) {
            connections.push_back(Connection(&socketMaster));
            continue;
        }

        /* the sorting is done because the apk connection always should be the first executed. this ensures the message
         * is received and can be forwarded to all other applications*/
        if (false == l_isVectorSorted) {
            std::sort(connections.begin(), connections.end(),
                      [](auto &lhs, auto &rhs) { return lhs.getConnectionType() < rhs.getConnectionType(); });
        }

        TCPConnMsgType message;
        for (auto &conn : connections) {
            conn.executeAssociatedFunction(&message);
        }

        if (message[0] == 255) {
            break;
        }
    }
}

int main(void) {
    startMasterServer();

    return 0;
}
