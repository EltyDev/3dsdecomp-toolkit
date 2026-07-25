#ifndef WRITABLE_HPP_
#define WRITABLE_HPP_

#include <fstream>

class Writable {
    public:
        virtual void write(std::ofstream& output) const = 0;
    protected:
    private:
};


#endif /* !WRITABLE_HPP_ */
