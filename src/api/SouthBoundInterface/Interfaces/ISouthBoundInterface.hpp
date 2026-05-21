#include <iostream>
#include <string>
#include <stdint.h>

class ISouthBoundInterface{
public: 
    virtual ~ISouthBoundInterface() = default;
    virtual bool connect(uint32_t ip, uint8_t port) = 0;
    virtual int sendData(const char* data, int len) = 0;
    virtual int receiveData(char* buffer, int len) = 0;
};

