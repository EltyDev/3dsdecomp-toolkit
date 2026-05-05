#include "parser/cxi.hpp"
#include "parser/function_heuristic.hpp"

int main(int argc, char* argv[])
{
    CXI cxi("../../test/ac.3ds");
    auto functions = function_heuristic::findFunctions(cxi.getTextSection(), cxi.getHeader().extendedHeader.systemControlInfo.text.address);
    return 0;
}

