#include "SBI.hpp"
#include "../Utils/validators/BodyValidator.hpp"

//Uses a map to associate OFPacketType enums with validators.
//when a packet type needs to be validated
//we lookup up the validation function aka the 'validator' in the map using the packet type 
//as the key, and the lambda func is called.
//
// --- Design Choice: Polymorphism vs. Dispatchers ---
// While polymorphism (using virtual functions) is a common C++ pattern, 
// a "Dispatcher" with a map and lambdas is often preferred in high-performance 
// networking for its flexibility and lower overhead when types are fixed (like enum protocols).
// This pattern allows us to "inject" custom logic per-type without deep class hierarchies.
//
// what is ValidatorFn? 
// its defined in the header to return a boolean.
std::unordered_map<oflib::PacketType, ValidatorFn> packetValidators = {
    //Packet type , Validator function (lambda that validates the packet).
    {oflib::PacketType::HELLO, [](const uint8_t* data, size_t len) { 
        // HELLO validation: Basic structural check + logging
        std::cout << "[Validator] Performing multi-stage check for HELLO..." << std::endl;
        bool isValid = Validator::BodyValidator::validateHello(data, static_cast<int>(len));
        
        if (isValid) {
            // Stage 2: Any additional heuristics or security checks could go here.
        }
        return isValid;
    }},
    {oflib::PacketType::PACKET_IN, [](const uint8_t* data, size_t len) { 
        // PACKET_IN validation: Deep structural check + Security Alerting
        std::cout << "[Validator] Performing multi-stage check for PACKET_IN..." << std::endl;
        if (!Validator::BodyValidator::validatePacketIn(data, static_cast<int>(len))) {
            return false;
        }

        // Stage 2: Heuristic check - if the body is surprisingly small, log a warning
        if (len < 20) {
            std::cout << "[Validator] Warning: PACKET_IN body is unusually small (" << len << " bytes)." << std::endl;
        }
        return true; 
    }},
    {oflib::PacketType::ERROR, [](const uint8_t* data, size_t len) {
        // ERROR validation: Check for minimal size and log the error code if possible
        std::cout << "[Validator] Performing multi-stage check for ERROR..." << std::endl;
        return Validator::BodyValidator::validateError(data, static_cast<int>(len));
    }},
    {oflib::PacketType::FLOW_MOD, [](const uint8_t* data, size_t len) {
        // FLOW_MOD validation: Critical for security as it modifies the flow table.
        // We call the detailed structural validator in BodyValidator.
        std::cout << "[Validator] Performing multi-stage check for FLOW_MOD..." << std::endl;
        return Validator::BodyValidator::validateFlowMod(data, static_cast<int>(len));
    }}
};

//could implement either inheritance or compositon, to avoid 
//std::cout << "Validator ....." all the time.