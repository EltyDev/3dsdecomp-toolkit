#include "parser/cxi.hpp"
#include "parser/function_heuristic.hpp"
#include <fstream>

int main(int argc, char* argv[])
{
    CXI cxi("../../test/ac.3ds");
    auto functions = function_heuristic::findFunctions(cxi.getTextSection(), cxi.getHeader().extendedHeader.systemControlInfo.text.address);
    std::ofstream output("functions.txt");
    for (const auto& function : functions) {
        output << std::hex << function.start << " " << std::hex << function.end << " " << (function.mode == CS_MODE_ARM ? "ARM" : "THUMB") << std::endl;
    }
    return 0;
}

