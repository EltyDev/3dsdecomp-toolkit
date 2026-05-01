#include "parser/function_heuristic.hpp"
#include <fstream>

int main(int argc, char* argv[])
{
    std::ifstream file("../../test/code.elf", std::ios::binary);
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    auto functions = function_heuristic::findFunctions(data);
    return 0;
}

