
struct OpenFlowHeader{
    uint8_t VERSION;
    uint8_t TYPE;
    uint32_t LENGTH;
    uint32_t XID;
};

struct PacketInMetaData{
    uint32_t BUFF_ID;
    uint16_t TOTAL_LEN;
    uint8_t REASON;
    uint8_t TABLE_ID;
    uint64_t COOKIE;
    int DATA;
};

struct PacketOutMetaData{

};

//when parsing bytes:
    //OpenFlowHeader* h = (OpenFlowHeader*)buffer; //risky

    //OpenFlowHeader parse_header(const uint8_t* buf); //preferred