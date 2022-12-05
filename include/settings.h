#include <iostream>
#include <libconfig.h++>

using namespace libconfig;

class settings {
    public:
        settings(std::string file);
        ~settings() 
        {
            if(cfg) 
            {
                delete cfg;
            }
        };
        std::string getValue(std::string name);
    private:
        Config* cfg;
};