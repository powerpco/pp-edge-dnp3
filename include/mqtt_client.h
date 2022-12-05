#include <iostream>

#include "mosquitto.h"
#include <tahu.h>
#include <tahu.pb.h>
#include <pb_decode.h>
#include <pb_encode.h>

#define DEFAULT_ADDR "127.0.0.1"
#define DEFAULT_PORT "1883"

using namespace std;
class mqtt_client {
    public:
        mqtt_client() {
            // Set default values
			m_host = DEFAULT_ADDR;
            m_port = (short unsigned int)atoi(DEFAULT_PORT);

            //libmosquitto initialization
            mosquitto_lib_init();

            //Create new libmosquitto client instance
            mosq = mosquitto_new(NULL, true, NULL);

            if (!mosq)
            {
                std::cout << "Error: failed to create mosquitto client" << std::endl;
            }
        };
        ~mqtt_client()
		{
			mosquitto_destroy(mosq);
            mosquitto_lib_cleanup();
		};
        bool connect();
        void publish(uint8_t* binary_buffer, size_t message_length);
        void setPort(uint16_t port)
		{
			m_port = port;
		};
		uint16_t getPort() { return m_port; };
        void setHost(string host)
        {
            m_host = host;
        }
        string getHost() { return m_host; };

        void setTopic(string topic)
        {
            m_topic = topic;
        }
        string getTopic() { return m_topic; };

    private:
        struct mosquitto *mosq;
        string m_host;
        short unsigned int m_port;
        string m_topic;
};