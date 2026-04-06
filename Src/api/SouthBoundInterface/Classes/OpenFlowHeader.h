
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

enum class OFPacketType : uint8_t {
    HELLO = 0,
    FEATURE_REQUEST = 1,
    PACKET_IN = 10
};

struct OFPacket {
    virtual void process() = 0;
    virtual ~OFPacket() = default;
};

struct OFPacketHello : OFPacket {
    void process() override {
        std::cout << "Processing HELLO packet" << std::endl;
    }
};

struct OFPacketIn : OFPacket {
    void process() override {
        //look up in the flow table?
       // int bytesSent = send();
        std::cout << "Processing PACKET_IN packet" << std::endl;
    }
};