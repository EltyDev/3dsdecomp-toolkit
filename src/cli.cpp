#include "parser/cxi.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
    CXI cxi("../../test/ac.3ds");
    std::cout << "Product Code: " << cxi.getHeader().header.productCode << std::endl;
    return 0;
}

