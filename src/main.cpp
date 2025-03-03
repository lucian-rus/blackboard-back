#include <algorithm>
#include <iostream>

#include "TCPConnection.h"

/**
 * @todo:
 ***        implement transmission lock
 *****      update handling for bad cases
 */

static TCPConnection socketMaster;
// @todo: update socket to handle different identification buffers
//// go from 255 to variable pattern of chars (for example 0xff/0xa5/0x5a/0xa5/0x5a/0xff)
// this helps with gibberish protection and frame integrity
// maybe add crc just for fun
// @todo: start working on tcp gibberish protection
// @todo: start working on frame integrity

void startMasterServer(void) {
    socketMaster.initServerSocket(_SOCKET_NUMBER);
    uint16_t tester_tx = 0xFFFF;
    uint16_t tester_rx = 0xFFFF;

#if defined(_CLIENT_CONN_LIMIT)
    socketMaster.setClientConnectionsLimit(_CLIENT_CONN_LIMIT);
#endif

    /* this is fine, as the data from tx should be sent to rx only if we receive it. */
#if defined(_ENABLE_TX_APP)
    tester_tx = socketMaster.acceptConnection(); // update this to only work from the apk -> front
#endif

#if defined(_ENABLE_RX_APP)
    tester_rx = socketMaster.acceptConnection();
#endif

    while (1) {
#if defined(_ENABLE_TX_APP)
        TCPConnMsgType message = socketMaster.read(tester_tx);
#endif

#if defined(_ENABLE_RX_APP)
        socketMaster.write(tester_rx, message);
#endif

        if (message[0] == 255) {
            break;
        }
    }
}

int main(void) {
    startMasterServer();

    return 0;
}
