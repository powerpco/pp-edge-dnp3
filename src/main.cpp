#include <signal.h>
#include <thread>

#include "dnp3.h"

static bool running = true;

void sigint_handler(int signalId)
{
    running = false;
}

using namespace libconfig;

int main(int argc, char* argv[])
{
    /* Add Ctrl-C handler */
    signal(SIGINT, sigint_handler);
   
    string filePath;
    if(argc>=2)
    {
        
        filePath = argv[1];
    }
    else
    {
        filePath = "/etc/pp-edge-dnp3/settings.cfg";
    }

    //cout << filePath << endl;

    settings* s = new settings(filePath);

    DNP3* handler = new DNP3();
    handler->configure(s);
    if(s) 
    {
        delete s;
    }

    handler->start();

    while(running)
    {
        sleep(120);
    }

    handler->stop();

    return EXIT_FAILURE;
}