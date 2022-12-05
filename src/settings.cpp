#include <iostream>
#include "settings.h"

using namespace std;
settings::settings(string file) 
{
    this->cfg = new Config();
    try
    {
        cfg->readFile(file.c_str());
    }
    catch(const FileIOException &fioex)
    {
        std::cerr << "I/O error while reading file." << std::endl;
    }
    catch(const ParseException &pex)
    {
        std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
                << " - " << pex.getError() << std::endl;
    }
}

string settings::getValue(string name)
{
    try
    {
        string val = this->cfg->lookup(name);
        return val;
    }
    catch(const SettingNotFoundException &nfex)
    {
        cerr << "No '" << name << "' setting in configuration file." << endl;
    }
    return "";
}