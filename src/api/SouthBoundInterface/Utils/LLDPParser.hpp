#ifndef LLDP_PARSER_HPP
#define LLDP_PARSER_HPP

#include <vector>
#include <string>
#include <stdint.h>
#include <cstring>

/**
 * LLDP TLV Types
 */
enum class LLDP_TLV_Type : uint8_t {
    END_OF_LLDPDU = 0,
    CHASSIS_ID = 1,
    PORT_ID = 2,
    TIME_TO_LIVE = 3,
    PORT_DESCRIPTION = 4,
    SYSTEM_NAME = 5,
    SYSTEM_DESCRIPTION = 6,
    SYSTEM_CAPABILITIES = 7,
    MANAGEMENT_ADDRESS = 8,
    ORGANIZATION_SPECIFIC = 127
};

struct LLDP_Neighbor {
    std::string chassis_id;
    std::string port_id;
    std::string system_name;
    uint16_t ttl;
};

class LLDPParser {
public:
    static bool isLLDP(const std::vector<uint8_t>& payload) {
        // Minimum Ethernet frame size for LLDP check
        // Dest MAC (6) + Src MAC (6) + EtherType (2)
        if (payload.size() < 14) return false;

        uint16_t etherType = (payload[12] << 8) | payload[13];
        return etherType == 0x88CC;
    }

    static LLDP_Neighbor parse(const std::vector<uint8_t>& payload) {
        LLDP_Neighbor neighbor;
        // Skip Ethernet header (14 bytes)
        size_t offset = 14;

        while (offset + 2 <= payload.size()) {
            uint16_t tlv_info = (payload[offset] << 8) | payload[offset + 1];
            uint8_t type = (tlv_info >> 9) & 0x7F;
            uint16_t length = tlv_info & 0x1FF;
            offset += 2;

            if (offset + length > payload.size()) break;

            switch (static_cast<LLDP_TLV_Type>(type)) {
                case LLDP_TLV_Type::CHASSIS_ID:
                    neighbor.chassis_id = std::string(reinterpret_cast<const char*>(&payload[offset + 1]), length - 1);
                    break;
                case LLDP_TLV_Type::PORT_ID:
                    neighbor.port_id = std::string(reinterpret_cast<const char*>(&payload[offset + 1]), length - 1);
                    break;
                case LLDP_TLV_Type::SYSTEM_NAME:
                    neighbor.system_name = std::string(reinterpret_cast<const char*>(&payload[offset]), length);
                    break;
                case LLDP_TLV_Type::TIME_TO_LIVE:
                    if (length == 2) {
                        neighbor.ttl = (payload[offset] << 8) | payload[offset + 1];
                    }
                    break;
                case LLDP_TLV_Type::END_OF_LLDPDU:
                    return neighbor;
                default:
                    break;
            }
            offset += length;
        }
        return neighbor;
    }
};

#endif // LLDP_PARSER_HPP
