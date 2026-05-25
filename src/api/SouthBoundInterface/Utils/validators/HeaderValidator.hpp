#ifndef HEADER_VALIDATOR_HPP
#define HEADER_VALIDATOR_HPP

#include "../../Classes/OpenFlowPackets.hpp"
#include <cstddef>
#include <iostream>

namespace Validator {

    /**
     * First-pass validation for OpenFlow packets.
     * Screens the header for basic protocol compliance.
     */
    inline bool validateHeader(const OpenFlowHeader& hdr, size_t receivedBytes) {
        const size_t MAX_OF_PACKET_SIZE = 65535; // Maximum OF 1.0 packet size

        // 1. Version Check: Support OpenFlow 1.0
        if (hdr.VERSION != 0x01) {
            std::cerr << "Security Alert: Unsupported OF Version (0x" 
                      << std::hex << (int)hdr.VERSION << ")" << std::endl;
            return false;
        }

        // 2. Minimum Length Check
        if (hdr.LENGTH < 8) {
            std::cerr << "Validation Failed: Packet too short (" << hdr.LENGTH << ")" << std::endl;
            return false;
        }

        // 3. Maximum Size Check (Prevent Buffer Overruns)
        if (hdr.LENGTH > MAX_OF_PACKET_SIZE) {
            std::cerr << "Security Alert: Packet length exceeds protocol limit (" << hdr.LENGTH << ")" << std::endl;
            return false;
        }

        // 4. Buffer Integrity
        if (hdr.LENGTH != receivedBytes) {
            std::cerr << "Validation Failed: Received " << receivedBytes << " but header expected " << hdr.LENGTH << std::endl;
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
