#pragma once

#include <stdint.h>
#include <vector>

class TCPMessage {
  public:
    // class
    TCPMessage();
    ~TCPMessage();

    /**
     * @brief Function to register the `header` of a TCP message. The header is defined as the first x bytes in the incoming frame which are used to validate it.
     *
     * @param[in] buffer
     * 
     * @return (void)
     */
    void registerCommStartHeader(const std::vector<uint8_t> &buffer);

  private:
    std::vector<uint8_t> m_InternalMessageBuffer; /** internal TCP message buffer */
    std::vector<uint8_t> m_commStartHeader;       /** buffer header */
};
