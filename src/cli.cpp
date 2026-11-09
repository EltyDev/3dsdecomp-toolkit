
#include "input/ida_export.hpp"
#include "output/ninja/build_rule.hpp"
#include "output/ninja/build_script.hpp"
#include "output/scatter/execution_region.hpp"
#include "output/scatter/input_section.hpp"
#include "output/scatter/load_region.hpp"
#include "parser/cxi.hpp"
#include "parser/function_heuristic.hpp"
#include <cstdint>
#include <filesystem>
#include "output/asm/asm_file.hpp"
#include "output/scatter/scatter_file.hpp"
#include <argparse/argparse.hpp>
#include "input/split/split_file.hpp"
#include <algorithm>

void splitRom(LoadRegion& codeRegion, LoadRegion& rodataRegion, LoadRegion& dataRegion, const std::vector<IDAFunction>& idaFunctions, SplitFile& splitFile, const std::vector<uint8_t> &data, const std::string& outputDir, ScatterFile& scatterFile)
{
    BuildScript buildScript;
    buildScript.addRule(Rule("compile_original", "armasm --cpu=MPCore -o $out $in", "Compiling $out"));
    buildScript.addRule(Rule("compile_new", "armcc -c --cpp --cpu=MPCore -O3 -Otime --fpmode=fast --forceinline $in -o $out ", "Compiling $out"));
    Rule linkRule("link", "armlink --sort=lexical --no_remove --scatter=scatter.sct --map --list=$out.map --output=$out --via=$rspfile", "Linking $out");
    linkRule.setRspFile("$out.rsp");
    linkRule.setRspFileContent("$in");
    buildScript.addRule(linkRule);
    BuildRule buildOriginal("program_original.elf", "link");
    BuildRule buildNew("program_new.elf", "link");
    for (auto& unit : splitFile.getUnits()) {
        std::string fileName = unit.getPathWithoutExtension();
        fileName = std::filesystem::path(fileName).filename().string();
        std::string outputFilePath = outputDir + "/orig/" + unit.getPathWithoutExtension() + ".s";
        std::filesystem::create_directories(std::filesystem::path(outputFilePath).parent_path());
        std::ofstream outputFile(outputFilePath);
        buildOriginal.addInput("build/orig/" + unit.getPathWithoutExtension() + ".o");
        buildNew.addInput("build/new/" +  unit.getPathWithoutExtension() + ".o");
        buildScript.addBuildRule(BuildRule("orig/" + unit.getPathWithoutExtension() + ".s", "build/orig/" + unit.getPathWithoutExtension() + ".o", "compile_original"));
        buildScript.addBuildRule(BuildRule("src/" + unit.getPath() ,"build/new/" + unit.getPathWithoutExtension() + ".o" , "compile_new"));
        ASMFile asmFile;
        std::vector<IDAFunction> functionsInUnit;
        for (const auto& function : idaFunctions) {
            if (function.location.start >= unit.getSections().front().getStart() && function.location.end <= unit.getSections().back().getEnd())
                functionsInUnit.push_back(function);
        }

        std::vector<SplitSection> sections = unit.getSections();
        std::sort(sections.begin(), sections.end(), [](const SplitSection& a, const SplitSection& b) {
            return a.getStart() < b.getStart();
        });
        for (auto& section : sections) {
            ExecutionRegion& region = codeRegion.getNearestExecutionRegion(section.getStart());
            if (section.getType() == SplitSection::Type::CODE) {
                std::vector<IDAFunction> functionsInSection;
                for (const auto& function : functionsInUnit) {
                    if (function.location.start >= section.getStart() && function.location.end <= section.getEnd())
                        functionsInSection.push_back(function);
                }
                size_t lastFunctionEnd = functionsInSection[0].location.start;
                for (const auto& function : functionsInSection) {
                    std::string name = std::format("{:x}", function.location.start);
                    ASMBlock block(
                        const_cast<uint8_t*>(data.data() + function.location.start), function.location.end - function.location.start, name, SplitSection::Type::CODE, function.location.mode, true
                    );
                    asmFile.addBlock(block);
                    region.addInputSection(InputSection(fileName + ".o", { "text.func_" + name }));
                    for (const auto& location : function.locations) {
                        std::string locName = std::format("{:x}", location.start);
                        ASMBlock locBlock(
                            const_cast<uint8_t*>(data.data() + location.start), location.end - location.start, locName, SplitSection::Type::CODE, location.mode, false
                        );
                        region.addInputSection(InputSection(fileName + ".o", { "text.loc_" + locName }));
                        asmFile.addBlock(locBlock);
                    }
                    if (function.location.end < lastFunctionEnd) {
                        std::string paddingName = std::format("{:x}", lastFunctionEnd);
                        ASMBlock paddingBlock(
                            const_cast<uint8_t*>(data.data() + function.location.end), lastFunctionEnd - function.location.end, paddingName, SplitSection::Type::DATA, function.location.mode, false
                        );
                        region.addInputSection(InputSection(fileName + ".o", { "data." + paddingName }));
                        asmFile.addBlock(paddingBlock);
                    }
                    lastFunctionEnd = function.location.end;
                }
            } else {
                std::string name = section.getName();
                ASMBlock block(
                    const_cast<uint8_t*>(data.data() + section.getStart()), section.getEnd() - section.getStart(), name, section.getType(), true, false
                );
                std::string prefix = section.getType() == SplitSection::Type::RODATA ? "rodata_" : "data_";
                region.addInputSection(InputSection(fileName + ".o", { prefix + name }));
                asmFile.addBlock(block);
            }
        }
        asmFile.write(outputFile);
        outputFile.close();
    }
    for (auto& region : codeRegion.getExecutionRegions())
        region.sortInputSectionsByStartAddress();
    scatterFile.addLoadRegion(codeRegion);
    scatterFile.addLoadRegion(rodataRegion);
    scatterFile.addLoadRegion(dataRegion);
    std::ofstream scatterOutput(outputDir + "/scatter.sct");
    scatterFile.write(scatterOutput);
    scatterOutput.close();
    std::ofstream buildOutput(outputDir + "/build.ninja");
    buildScript.addBuildRule(buildOriginal);
    buildScript.addBuildRule(buildNew);
    buildScript.write(buildOutput);
    buildOutput.close();
}

void splitFunctionsFromFile(const std::string &romFile, const std::string& splitFile, const std::string& idaFile, const std::string& outputDir)
{
    std::ifstream input(splitFile);
    std::ifstream idaInput(idaFile);
    CXI cxi(romFile);
    std::vector<uint8_t> totalData = cxi.getTotalData();
    if (!input.is_open()) {
        std::cerr << "Failed to open input file: " << splitFile << std::endl;
        return;
    }
    if (!idaInput.is_open()) {
        std::cerr << "Failed to open IDA export file: " << idaFile << std::endl;
        return;
    }
    SplitFile split;
    IDAExport idaExport;
    try {
        split.read(input);
        idaExport.read(idaInput);
    } catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
        return;
    }
    ScatterFile scatterFile;
    std::unique_ptr<LoadRegion> codeRegion;
    std::unique_ptr<LoadRegion> rodataRegion;
    std::unique_ptr<LoadRegion> dataRegion;
    std::vector<SplitSection> sections = split.getSections();
    std::sort(sections.begin(), sections.end(), [](const SplitSection& a, const SplitSection& b) {
        return a.getStart() < b.getStart();
    });
    for (const SplitSection &section : split.getSections()) {
        bool isBss = section.getType() == SplitSection::Type::BSS;
        ExecutionRegion region = ExecutionRegion(
            section.getName(),
            true,
            section.getStart(),
            isBss ? section.getEnd() - section.getStart() : 0
        );
        switch (section.getType()) {
            case SplitSection::Type::CODE:
                if (!codeRegion)
                    codeRegion = std::make_unique<LoadRegion>("LR_CODE", true, section.getStart());
                codeRegion->addExecutionRegion(region);
                break;
            case SplitSection::Type::RODATA:
                if (!rodataRegion)
                    rodataRegion = std::make_unique<LoadRegion>("LR_RODATA", true, section.getStart());
                rodataRegion->addExecutionRegion(region);
                break;
            case SplitSection::Type::DATA:
            case SplitSection::Type::BSS:
               if (isBss)
                    region.addAttribute(ExecutionRegion::Attribute::EMPTY);
                if (!dataRegion)
                    dataRegion = std::make_unique<LoadRegion>("LR_DATA", true, section.getStart());
                dataRegion->addExecutionRegion(region);
                break;
        }
    }
    splitRom(*codeRegion, *rodataRegion, *dataRegion, idaExport.getFunctions(), split, totalData, outputDir, scatterFile);

}

int main(int argc, char* argv[])
{
    argparse::ArgumentParser program("3dsdecomp-toolkit", "0.1.0");
    argparse::ArgumentParser split("split");
    split.add_argument("rom").help("Input ROM file");
    split.add_argument("input").help("Input splits file");
    split.add_argument("ida").help("Input IDA export file");
    split.add_argument("output").help("Output directory");
    program.add_subparser(split);
    try {
        if (argc == 1) {
            std::cout << program;
            return 0;
        }
        program.parse_args(argc, argv);
        if (program.is_subcommand_used("split")) {
            std::string romFile = split.get<std::string>("rom");
            std::string inputFile = split.get<std::string>("input");
            std::string idaFile = split.get<std::string>("ida");
            std::string outputDir = split.get<std::string>("output");
            try {
                splitFunctionsFromFile(romFile, inputFile, idaFile, outputDir);
            } catch (const std::exception& error) {
                std::cerr << error.what() << std::endl;
                return 1;
            }
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
        std::cerr << program;
        return 1;
    }
}

