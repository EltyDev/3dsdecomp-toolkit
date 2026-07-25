#include "output/ldscript/ldsection.hpp"
#include "output/ninja/build_rule.hpp"
#include "output/ninja/build_script.hpp"
#include "parser/cxi.hpp"
#include "parser/function_heuristic.hpp"
#include <filesystem>
#include "output/asm/asm_file.hpp"
#include "output/ldscript/ldscript.hpp"
#include <iostream>

void writeBinaryFile(const std::string& filename, uint8_t *data, size_t size,  cs_mode mode = CS_MODE_ARM, bool isData = false, const std::string &section = "text")
{
    std::ofstream output(filename + ".s");
    ASMFile asmFile;
    asmFile.addBlock(ASMBlock(data, size, isData ? "" : filename, mode == CS_MODE_THUMB));
    asmFile.write(output);
}

void splitFunctions(std::vector<Function> &functions, LDSection &textSection, BuildScript &buildScript, std::vector<uint8_t> &text, uint32_t baseAddress) {
    ASMFile asmfile;
    std::ofstream output;
    std::string firstName;
    uintptr_t lastAddress = 0;
    for (size_t i = 0; i < functions.size(); ++i) {
        const auto& function = functions[i];
        std::string name = std::format("{:x}", function.start);
        if (function.start > lastAddress && lastAddress != 0)
            asmfile.addBlock(ASMBlock(text.data() + (lastAddress - baseAddress), function.start - lastAddress, "", function.mode == CS_MODE_THUMB));
        if (i % 30 == 0) {
            if (i != 0) {
                asmfile.write(output);
                textSection.addFile("build/" + firstName, std::stoul(firstName, nullptr, 16));
                buildScript.addBuildRule(BuildRule("output/" + firstName + ".s", "build/" + firstName + ".o", "compile"));
                asmfile.reset();
                output.close();
            }
            firstName = name;
            output = std::ofstream("output/" + firstName + ".s");
        }
        asmfile.addBlock(ASMBlock(text.data() + (function.start - baseAddress), function.end - function.start, name, function.mode == CS_MODE_THUMB));
        lastAddress = function.end;
    }
    if (output.is_open()) {
        asmfile.write(output);
        textSection.addFile("build/" + firstName, std::stoul(firstName, nullptr, 16));
        buildScript.addBuildRule(BuildRule("output/" + firstName + ".s", "build/" + firstName + ".o", "compile"));
        asmfile.reset();
        output.close();
    }
}

int main(int argc, char* argv[])
{
    CXI cxi("../../test/ac.3ds");
    auto functions = function_heuristic::findFunctions(cxi.getTextSection(), cxi.getHeader().extendedHeader.systemControlInfo.text.address);
    std::filesystem::create_directory("output");
    uintptr_t lastAddress = 0;
    std::vector<uint8_t> &text = cxi.getTextSection();
    std::vector<uint8_t> &rodata = cxi.getRodataSection();
    std::vector<uint8_t> &data = cxi.getDataSection();
    writeBinaryFile("output/rodata", rodata.data(), rodata.size(), CS_MODE_ARM, true, "rodata");
    writeBinaryFile("output/data", data.data(), data.size(), CS_MODE_ARM, true, "data");
    std::ofstream ldfile("ldscript.ld");
    std::ofstream buildfile("build.ninja");
    LDScript ldscript("start");
    LDSection textSection(cxi.getHeader().extendedHeader.systemControlInfo.text.address, "text");
    LDSection dataSection(cxi.getHeader().extendedHeader.systemControlInfo.data.address, "data");
    LDSection rodataSection(cxi.getHeader().extendedHeader.systemControlInfo.rodata.address, "rodata");
    LDSection bssSection(cxi.getHeader().extendedHeader.systemControlInfo.data.address + cxi.getHeader().extendedHeader.systemControlInfo.data.size, "bss", cxi.getHeader().extendedHeader.systemControlInfo.bssSize);
    dataSection.addFile("build/data");
    rodataSection.addFile("build/rodata");
    BuildScript buildScript;
    buildScript.addRule(Rule("compile", "arm-none-eabi-as -mcpu=mpcore -o $out $in", "Compiling $out"));
    Rule linkRule("link", "arm-none-eabi-gcc -nostdlib -T ldscript.ld -Wl,-Map=$out.map -o $out @$rspfile", "Linking $out");
    linkRule.setRspFile("$out.rsp");
    linkRule.setRspFileContent("$in");
    buildScript.addRule(linkRule);
    buildScript.addBuildRule(BuildRule("output/data.s", "build/data.o", "compile"));
    buildScript.addBuildRule(BuildRule("output/rodata.s", "build/rodata.o", "compile"));
    BuildRule buildRule("program.elf", "link");
    buildRule.addInput("build/data.o");
    buildRule.addInput("build/rodata.o");
    uint32_t baseAddress = cxi.getHeader().extendedHeader.systemControlInfo.text.address;
    splitFunctions(functions, textSection, buildScript, text, baseAddress);
    buildScript.addBuildRule(buildRule);
    buildScript.write(buildfile);
    ldscript.addSection(textSection);
    ldscript.addSection(rodataSection);
    ldscript.addSection(dataSection);
    ldscript.addSection(bssSection);
    ldscript.write(ldfile);
    return 0;
}

