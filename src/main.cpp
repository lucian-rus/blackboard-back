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

    /* this is fine, as the data from tx should be sent to rx only if we receive it. */
    uint16_t tester_tx = socketMaster.acceptConnection();
    uint16_t tester_rx = socketMaster.acceptConnection();

    while (1) {
        TCPConnMsgType message = socketMaster.read(tester_tx);
        socketMaster.write(tester_rx, message);

        if (message[0] == 255) {
            break;
        }
    }
}

int main(void) {
    startMasterServer();

    return 0;
}
