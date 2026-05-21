#ifndef HEADER_VALIDATOR_HPP
#define HEADER_VALIDATOR_HPP

#include "../../Classes/OpenFlowPackets.hpp"
#include <cstddef>
#include <iostream>

namespace Validator {

    /**
     * First-pass validation for OpenFlow packets.
     * Screens the header for basic protocol compliance before dispatching to factories.
     * 
     * @param hdr The parsed OpenFlow header.
     * @param receivedBytes The actual number of bytes read from the socket.
     * @return true if the packet passes basic sanity checks, false otherwise.
     */
    inline bool validateHeader(const OpenFlowHeader& hdr, size_t receivedBytes) {
        // 1. Version Check: Support OpenFlow 1.0 (0x01)
        if (hdr.VERSION != 0x01) {
            std::cerr << "Validation Failed: Unsupported OF Version (0x" 
                      << std::hex << (int)hdr.VERSION << ")" << std::endl;
            return false;
        }

        // 2. Minimum Length Check: Header is 8 bytes, so length must be >= 8
        if (hdr.LENGTH < 8) {
            std::cerr << "Validation Failed: Packet length too short (" 
                      << hdr.LENGTH << " bytes)" << std::endl;
            return false;
        }

        // 3. Buffer Integrity: Does the length field match the data we actually received?
        if (hdr.LENGTH != receivedBytes) {
            std::cerr << "Validation Failed: Header length (" << hdr.LENGTH 
                      << ") does not match received bytes (" << receivedBytes << ")" << std::endl;
            return false;
        }

        // 4. Type Sanity: Ensure the type falls within the defined v1.0 range (0-21)
        // Note: This prevents passing invalid types to the factory map.
        if (hdr.TYPE > 21) {
            std::cerr << "Validation Failed: Unknown Packet Type (" << (int)hdr.TYPE << ")" << std::endl;
            return false;
        }

        return true;
    }
}

#endif // HEADER_VALIDATOR_HPP
