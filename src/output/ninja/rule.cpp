#include "output/ninja/rule.hpp"

Rule::Rule(const std::string &name, const std::string &command, const std::string &description) : _name(name), _command(command), _description(description) {}

void Rule::write(std::ofstream& output) const {
    output << "rule " << _name << "\n";
    output << "  command = " << _command << "\n";
    if (!rspfile.empty())
        output << "  rspfile = " << rspfile << "\n";
    if (!rspfile_content.empty())
        output << "  rspfile_content = " << rspfile_content << "\n";
    if (!_description.empty())
        output << "  description = " << _description << "\n";
}

const std::string& Rule::getName() const {
    return _name;
}

const std::string& Rule::getCommand() const {
    return _command;
}

const std::string& Rule::getDescription() const {
    return _description;
}

const std::string& Rule::getRspFile() const {
    return rspfile;
}

const std::string& Rule::getRspFileContent() const {
    return rspfile_content;
}

void Rule::setRspFile(const std::string &rspfile) {
    this->rspfile = rspfile;
}

void Rule::setRspFileContent(const std::string &rspfile_content) {
    this->rspfile_content = rspfile_content;
}