#include "IMessage.hpp"
#include <stdint.h>

class OpenFlowMessage:IMessage{
    uint8_t TYPE;

    void process() override {
        
    }
};