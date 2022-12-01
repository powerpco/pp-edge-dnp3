#include <signal.h>
#include <thread>

#include "dnp3.h"

static bool running = true;

void sigint_handler(int signalId)
{
    running = false;
}

int main(int argc, char* argv[])
{
    /* Add Ctrl-C handler */
    signal(SIGINT, sigint_handler);
    
    /*
    mqtt_client c = mqtt_client();
    c.setHost("localhost");
    c.setPort(1883);
    c.connect();
    */

    DNP3* handler = new DNP3();
    handler->configure();
    handler->start();

    while(running)
    {
        sleep(120);
    }

    handler->stop();

    return 0;
}