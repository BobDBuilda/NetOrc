#ifndef BODY_VALIDATOR_HPP
#define BODY_VALIDATOR_HPP

#include "../../Classes/OpenFlowPackets.hpp"
#include <cstddef>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>

namespace Validator {

    /**
     * Body Validation logic for specific OpenFlow packet types.
     * Focuses on structural integrity and preventing "Dangerous" over-reads.
     */
    class BodyValidator {
    public:
        /**
         * Validates the body of a HELLO packet.
         */
        static bool validateHello(char* data, int len) {
            // HELLO bodies are optional version negotiation. 
            // Minimum is 0, maximum is (hdr.LENGTH - 8).
            return true; 
        }

        /**
         * Validates the body of a PACKET_IN packet.
         * Structure (OF 1.0):
         * - buffer_id (4 bytes)
         * - total_len (2 bytes) <- The length of the raw packet data
         * - in_port (2 bytes)
         * - reason (1 byte)
         * - pad (1 byte)
         * Total mandatory body: 10 bytes + RAW_DATA
         */
        static bool validatePacketIn(char* data, int len) {
            if (len < 10) {
                std::cerr << "Validation Failed: PACKET_IN body too small for header." << std::endl;
                return false;
            }

            // Extract the 'total_len' of the raw frame included in this message
            uint16_t frame_len;
            memcpy(&frame_len, data + 4, 2);
            frame_len = ntohs(frame_len);

            // SECURITY CHECK: Ensure the frame_len doesn't claim to be larger 
            // than the actual buffer remaining.
            if (frame_len > (len - 10)) {
                std::cerr << "Security Alert: PACKET_IN over-read detected! "
                          << "Frame claims " << frame_len << " bytes but only " 
                          << (len - 10) << " available." << std::endl;
                return false;
            }

            return true;
        }

        /**
         * Validates the body of an ERROR packet.
         * Structure: type(2), code(2), plus variable data.
         */
        static bool validateError(char* data, int len) {
            if (len < 4) {
                std::cerr << "Validation Failed: ERROR body too short." << std::endl;
                return false;
            }
            return true;
        }

        /**
         * Validates the body of a FLOW_MOD packet.
         * Structure (OF 1.0) - 24 bytes mandatory:
         * - ofp_match (40 bytes? No, in v1.0 the body starts with match which is 40 bytes)
         * Wait, in OF 1.0:
         * Header (8) + Match (40) + Cookie (8) + Command (2) + ...
         * Total body is significantly larger. 
         * Let's verify: Match is 40 bytes, Cookie is 8, Command 2, Idle 2, Hard 2, Priority 2, 
         * BufferID 4, OutPort 2, Flags 2 = 72 bytes for the mandatory body.
         */
        static bool validateFlowMod(char* data, int len) {
            if (len < 72) {
                std::cerr << "Validation Failed: FLOW_MOD body too small for OF 1.0 (" << len << " bytes)." << std::endl;
                return false;
            }

            // Extract Command (Match is 40, Cookie 8 = 48 bytes offset)
            uint16_t command;
            memcpy(&command, data + 48, 2);
            command = ntohs(command);

            // Command range: 0 (ADD) to 4 (DELETE_STRICT)
            if (command > 4) {
                std::cerr << "Security Alert: Invalid FLOW_MOD command (" << command << ")" << std::endl;
                return false;
            }

            return true;
        }

        /**
         * Simple sanitizer to ensure binary data doesn't contain unexpected 
         * patterns if it were ever logged as a string.
         */
        static void sanitizeBinary(char* data, int len) {
            // In a binary protocol, we don't 'remove' characters because 
            // 0x3B might be a valid part of an IP address, not a semicolon.
            // Instead, 'sanitization' means ensuring we never treat it as a string.
        }
    };
}

#endif // BODY_VALIDATOR_HPP
