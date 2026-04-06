#include "IHeader.hpp"
#include "IBody.hpp"

class IMessage{
    IHeader hdr;
    IBody bdy;
    int payload;
public:
    void virtual process() = 0;
};