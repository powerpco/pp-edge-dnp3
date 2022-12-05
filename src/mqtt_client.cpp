#include "mqtt_client.h"

bool mqtt_client::connect() 
{
    //Connect to MQTT broker
    if (mosquitto_connect(this->mosq, this->getHost().c_str(), this->getPort(), 60) != MOSQ_ERR_SUCCESS)
    {
	    std::cout << "Error: connecting to MQTT broker failed" << std::endl;
        return false;
    }
    return true;
}

void mqtt_client::publish(uint8_t* binary_buffer, size_t message_length) 
{
    mosquitto_publish(this->mosq, NULL, this->getTopic().c_str(), message_length, binary_buffer, 0, false);
}