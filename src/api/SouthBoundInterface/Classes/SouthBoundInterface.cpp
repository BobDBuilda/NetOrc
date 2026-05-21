#include "SouthBoundInterface.hpp"

//Uses a map to associate OFPacketType enums with factories.
//when a apcket type needs to be created
//we lookup up the creation function aka the 'factory' in the map using the packet type 
//as the key, and the lambda func is called.
//what is FactoryFn? 
//its defined in the header to point to return std::unique_ptr<OFPacket>.
std::unordered_map<OFPacketType, FactoryFn> packetFactories = {
    //Packet type , Factory function (lambda that creates the packet).
    {OFPacketType::HELLO, [](char*, int) { return std::make_unique<OFPacketHello>(); }}, //return a ptr to OFPacketHello
    {OFPacketType::PACKET_IN, [](char*, int) { return std::make_unique<OFPacketIn>(); }} //return a ptr to OFPacketIn
};
