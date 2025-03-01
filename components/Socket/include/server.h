#pragma once

/** @todo:
 * update to be configurable
 *    * supports TCPMessage LIB
 *    * support internal buffer -> limited functionality
 *    * define support for logger -> enable/disable
 *    * define support for external error manager -> enable/disable
 */
#include <stdint.h>

#include <queue>
#include <vector>

// VHALSocket module status codes
#define VHALSOCKET_STATUS_OK                  0 /** no errors during function execution */
#define WHALSOCKET_STATUS_ALREADY_INIT        1 /** module already init */
#define VHALSOCKET_STATUS_WINSOCKET_INIT_FAIL 2 /** winsocket init failed (windows only) */
#define VHALSOCKET_STATUS_SOCKET_INIT_FAIL    3 /** socket init failed */

#define VHALSOCKET_STATUS_SERVER_INIT_FAIL 4 /** server socket init failed */
#define VHALSOCKET_STATUS_CLIENT_INIT_FAIL 5 /** client socket init failed */
#define VHALSOCKET_STATUS_SERVER_NOT_INIT  6 /** server not initialized. must init server first */
#define VHALSOCKET_STATUS_CLIENT_NOT_INIT  7 /** client not initialized. must init client first */

#define VHALSOCKET_STATUS_SOCKET_BIND_FAIL  8  /** socket bind failed (server only) */
#define VHALSOCKET_STATUS_SOCKET_START_FAIL 9  /** socket start failed (server only) */
#define VHALSOCKET_STATUS_SOCKET_ERROR      10 /** socket encountered an error */
#define VHALSOCKET_STATUS_SOCKET_CLOSED     11 /** socket connection closed */

// VHALSocket general use macros
#define VHALSOCKET_MAX_CHARACTER_COUNTER 253 /** maximum number of characters allowed during a transmission (2 reserved) */
#define VHALSOCKET_BUFFER_START_CHAR     255 /** signal the VHALSocket that a new valid transmission is happening */
#define VHALSOCKET_BUFFER_START_PADDING  2   /** transmission buffer padding (consumes the 2 reserved bytes) */

class VHALSocket {
  public:
    typedef struct _CallbackParam {
        void    *param; /** pointer to param */
        uint32_t paramType /** type of param */;
    } CallbackParam_t;

    // class
    VHALSocket();
    ~VHALSocket();

    int32_t initServer(const int &port);
    int32_t initClient(const int &port);
    int32_t deinit(void);

    int32_t startServer(void);
    int32_t startClient(void);
    int32_t stop(void);

    int32_t write(const std::vector<uint8_t> &buffer);
    int32_t read(std::vector<uint8_t> &buffer);

    int32_t registerReadCallback(void (*callbackFunction)(std::vector<uint8_t>));
    int32_t registerWriteCallback(void (*callbackFunction)());

  private:
    bool m_ShouldServerRun; /** signals if the server main loop should run */
    bool m_ShouldClientRun; /** signals if the client main loop should run */

    bool m_ConnectionEstablished;

    // internal soscket handlers -> update to smart pointers
    void *m_ConnectionSocket; /** holds the connection socket (for both server and client use) */
    void *m_Socket;           /** holds the socket that manages r/w operations */

    // internal callback handlers -> maybe try doing this in a better way. this should be updated to be
    void (*readCallback)(std::vector<uint8_t>); /** calback to read function (optional) */
    void (*writeCallback)();                    /** calback to write function (optional) */

    // internal queue that holds all messages
    std::queue<std::vector<uint8_t>> m_Queue; /** queue that holds all retrieved messages */

    // internal functions
    int32_t initSocket(void);
    // update this to vector
    int32_t readSocketBuffer(uint8_t *buffer);
};
