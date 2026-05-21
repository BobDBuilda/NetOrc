#include <iostream>
#include "core/Classes/App.h"
#include "api/NorthBoundInterface/NorthBoundInterface.hpp"
#include "api/SouthBoundInterface/Classes/SouthBoundInterface.hpp"

int main() {
    NorthBoundInterface nbi;
    SouthBoundInterface sbi;

    App app(nbi, sbi);
    app.run();

    return 0;
}
