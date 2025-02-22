#include <algorithm>
#include <iostream>

#ifdef _LINUXPLATFORM
    #include <thread>
#endif

#include "server.h"

/**
 * @todo:
 ***        implement transmission lock
 ***        implement read check for null queue (measure of safety for no callbacks)
 ******     make this work with gnu -> mostly works
 **         check if logs are all good
 ***        check for memory leaks
 *****      update handling for bad cases
 */

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
// @todo: update socket to handle different identification buffers
//// go from 255 to variable pattern of chars (for example 0xff/0xa5/0x5a/0xa5/0x5a/0xff)
// this helps with gibberish protection and frame integrity
// maybe add crc just for fun
// @todo: start working on tcp gibberish protection
// @todo: start working on frame integrity
void startMasterServer(void) {
    socketMaster.initServer(_APK_CONN);
    // @todo: add protections
    socketMaster.registerReadCallback(callbackFn);

    socketMaster.startServer();
}

void startSlaveServer(void) {
    socketSlave.initServer(_APP_CONN);
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
