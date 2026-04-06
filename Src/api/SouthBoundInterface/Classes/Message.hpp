#include "IHeader.hpp"
#include "IBody.hpp"

class Message{
    IHeader hdr;
    IBody bdy;
    int payload;
public:
    void process(){};
};