#include <iostream>

class OpenFlowHeader{
    int VERSION = 0;
    int TYPE = 0;
    int LENGTH = 0;
    int XID = 0;

public:
    OpenFlowHeader(){
        std::cout << "woo" << std::endl;
    }
};