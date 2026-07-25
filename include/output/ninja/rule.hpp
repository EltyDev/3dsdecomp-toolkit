#ifndef RULE_HPP_
#define RULE_HPP_

#include "../writable.hpp"

class Rule : public Writable {
    public:
        Rule(const std::string &name, const std::string &command, const std::string &description);
        ~Rule() = default;
        void write(std::ofstream& output) const override;
        const std::string& getName() const;
        const std::string& getCommand() const;
        const std::string& getDescription() const;
        const std::string& getRspFile() const;
        const std::string& getRspFileContent() const;
        void setRspFile(const std::string &rspfile);
        void setRspFileContent(const std::string &rspfile_content);
    protected:
    private:
        std::string _name;
        std::string _command;
        std::string _description;
        std::string rspfile;
        std::string rspfile_content;
};


#endif /* !RULE_HPP_ */
