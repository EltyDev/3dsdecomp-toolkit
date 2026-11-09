#ifndef READABLE_HPP_
#define READABLE_HPP_

#include <fstream>

class Readable {
    public:
        virtual void read(std::ifstream& input, const std::string &line = "") = 0;
    protected:
    private:
};

#endif /* !READABLE_HPP_ */