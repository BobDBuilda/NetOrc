#include <stdint.h>

struct OpenFlowHeader1
{
    uint8_t version;
    uint8_t type;
    uint16_t length;
    uint32_t xid;

    //when parsing bytes:
    //OpenFlowHeader* h = (OpenFlowHeader*)buffer; //risky

    //OpenFlowHeader parse_header(const uint8_t* buf); //preferred


};
