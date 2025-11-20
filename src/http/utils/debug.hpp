#pragma once
#include <iostream>

#ifdef DEBUG
    #define DBG(x) std::cout << x << std::endl
#else
    #define DBG(x) // nothing
#endif