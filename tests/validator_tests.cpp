#include <cstdlib>
#include <iostream>

#include "api/SouthBoundInterface/Utils/validators/HeaderValidator.hpp"
#include "api/SouthBoundInterface/Utils/validators/BodyValidator.hpp"

namespace {

int g_failed = 0;

void expectTrue(bool value, const char* name) {
    if (!value) {
        std::cerr << "[FAIL] " << name << std::endl;
        ++g_failed;
    } else {
        std::cout << "[PASS] " << name << std::endl;
    }
}

void expectFalse(bool value, const char* name) {
    expectTrue(!value, name);
}

OpenFlowHeader makeHeader(uint8_t version, uint8_t type, uint16_t length, uint32_t xid = 1) {
    return OpenFlowHeader{version, type, length, xid};
}

} // namespace

int main() {
    using Validator::BodyValidator;
    using Validator::validateHeader;

    {
        const auto hdr = makeHeader(0x01, 0, 8);
        expectTrue(validateHeader(hdr, 8), "validateHeader accepts a valid HELLO header");
    }

    {
        const auto hdr = makeHeader(0x02, 0, 8);
        expectFalse(validateHeader(hdr, 8), "validateHeader rejects unsupported version");
    }

    {
        const auto hdr = makeHeader(0x01, 0, 7);
        expectFalse(validateHeader(hdr, 7), "validateHeader rejects packets shorter than 8 bytes");
    }

    {
        const auto hdr = makeHeader(0x01, 0, 16);
        expectFalse(validateHeader(hdr, 12), "validateHeader rejects length mismatches");
    }

    {
        const auto hdr = makeHeader(0x01, 0, 65535);
        expectTrue(validateHeader(hdr, 65535), "validateHeader accepts the maximum representable packet size");
    }

    {
        const auto hdr = makeHeader(0x01, 22, 8);
        expectFalse(validateHeader(hdr, 8), "validateHeader rejects unknown packet types");
    }

    {
        char data[1] = {};
        expectTrue(BodyValidator::validateHello(data, 0), "validateHello always accepts empty body");
    }

    {
        char data[9] = {};
        expectFalse(BodyValidator::validatePacketIn(data, 9), "validatePacketIn rejects bodies smaller than 10 bytes");
    }

    {
        char data[14] = {};
        data[4] = 0x00;
        data[5] = 0x10;
        expectFalse(BodyValidator::validatePacketIn(data, 14), "validatePacketIn rejects over-read claims");
    }

    {
        char data[20] = {};
        data[4] = 0x00;
        data[5] = 0x04;
        expectTrue(BodyValidator::validatePacketIn(data, 20), "validatePacketIn accepts valid body");
    }

    {
        char data[3] = {};
        expectFalse(BodyValidator::validateError(data, 3), "validateError rejects short bodies");
    }

    {
        char data[4] = {};
        expectTrue(BodyValidator::validateError(data, 4), "validateError accepts minimum body");
    }

    {
        char data[71] = {};
        expectFalse(BodyValidator::validateFlowMod(data, 71), "validateFlowMod rejects short bodies");
    }

    {
        char data[72] = {};
        data[48] = 0x00;
        data[49] = 0x05;
        expectFalse(BodyValidator::validateFlowMod(data, 72), "validateFlowMod rejects invalid commands");
    }

    {
        char data[72] = {};
        data[48] = 0x00;
        data[49] = 0x04;
        expectTrue(BodyValidator::validateFlowMod(data, 72), "validateFlowMod accepts valid command range");
    }

    if (g_failed != 0) {
        std::cerr << g_failed << " test(s) failed" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "All validator tests passed" << std::endl;
    return EXIT_SUCCESS;
}
