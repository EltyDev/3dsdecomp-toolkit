#include "parser/cxi.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
    CXI cxi("../../test/ac.3ds");
    std::cout << "Product Code: " << cxi.getHeader().header.productCode << std::endl;
    std::cout << "Text Section Size: " << cxi.getTextSection().size() << std::endl;
    for (size_t i = 0; i < std::min(cxi.getTextSection().size(), static_cast<size_t>(16)); i++) {
        std::cout << std::hex << static_cast<int>(cxi.getTextSection()[i]) << " ";
    }
    std::cout << std::endl;
    return 0;
}

