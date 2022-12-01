#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string>
#include <time.h>

#include "dnp3.h"
#include <tahu.h>
#include <tahu.pb.h>
#include <pb_decode.h>
#include <pb_encode.h>

using namespace std;
using namespace opendnp3;
bool DNP3::start()
{
    uint16_t masterId = this->getMasterLinkId();
	bool scanEnabled = this->isScanEnabled();
	unsigned long applicationTimeout = this->getTimeout();
	unsigned long scanInterval = this->getOutstationScanInterval();
	OutStationTCP *outstation = m_outstation;

	cout << "configured DNP3 master: " << masterId << " and app timeout" << applicationTimeout << endl;

    DNP3Manager* manager = new DNP3Manager(1, ConsoleLogger::Create());
    m_manager = manager;

    string remoteLabel = "remote_" + to_string(outstation->linkId);

	const auto logLevels = levels::NOTHING | flags::WARN | flags::ERR;

	std::shared_ptr<IChannel> channel = manager->AddTCPClient(
        remoteLabel, 
		logLevels, 
		ChannelRetry::Default(), 
		{ IPEndpoint(outstation->address, outstation->port) }, 
		"0.0.0.0",
		PrintingChannelListener::Create()
    );

	if (!channel)
	{
		return false;
	}

    cout << "configured DNP3 TCP outstation is: " << outstation->address.c_str() << ":" << outstation->port << ", Link Id " << outstation->linkId << endl;

	MasterStackConfig stackConfig;
	stackConfig.master.responseTimeout = TimeDuration::Seconds(applicationTimeout);
    stackConfig.master.disableUnsolOnStartup = true;
	//stackConfig.master.startupIntegrityClassMask = ClassField::None();
	stackConfig.link.LocalAddr = masterId;
	stackConfig.link.RemoteAddr = outstation->linkId;

	std::shared_ptr<ISOEHandler> SOEHandle = std::make_shared<DNP3SOEHandler>(this, remoteLabel);
	if (!SOEHandle)
	{
		return false;
	}

	std::shared_ptr<IMaster> master = channel->AddMaster(
        "master_" + to_string(masterId),
        SOEHandle, 
        DefaultMasterApplication::Create(), 
        stackConfig
    );

	if (!master)
	{
		return false;
	}

	// Do an integrity poll (Class 3/2/1/0) once per specified seconds
	if (scanEnabled)
	{
		cout << "Outstation scan (Integrity Poll) is enabled" << endl;
		master->AddClassScan(ClassField::AllClasses(), TimeDuration::Seconds(scanInterval), SOEHandle);
		//auto integrityScan = master->AddClassScan(ClassField::AllClasses(), TimeDuration::Minutes(1), SOEHandle);
		//auto exceptionScan = master->AddClassScan(ClassField(ClassField::CLASS_1), TimeDuration::Seconds(5), SOEHandle);

	}

	this->m_client = mqtt_client();
	this->m_client.connect();

	// Enable the DNP3 master and connect to outstation
	return master->Enable();

}

template<class T> void DNP3SOEHandler::DNP3DataCallback(const HeaderInfo& info, const ICollection<Indexed<T>>& values, const string& objectType)
{
	this->ddata_payload = new org_eclipse_tahu_protobuf_Payload();
    get_next_payload(this->ddata_payload);
	cout << "callback" << endl;

    auto processData = [&](const Indexed<T>& pair)
	{
		this->dataElement<T>(info, pair.value, pair.index, objectType);
	};
    values.ForeachItem(processData);

    if(this->ddata_payload->metrics_count > 0) {
        // Encode the payload into a binary format so it can be published in the MQTT message.
        // The binary_buffer must be large enough to hold the contents of the binary payload
        size_t buffer_length = 64*1024;
        uint8_t *binary_buffer = (uint8_t *)malloc(buffer_length * sizeof(uint8_t));
        size_t message_length = encode_payload(binary_buffer, buffer_length, this->ddata_payload);

        // Publish the DDATA on the appropriate topic
        //mosquitto_publish((*data).mosq, NULL, (*data).DDATA, message_length, binary_buffer, 0, false);
		this->m_dnp3->send(binary_buffer, message_length);

        // Free the memory
        free(binary_buffer);
    }

    free_payload(this->ddata_payload);
}

template<class T> void DNP3SOEHandler::dataElement(const HeaderInfo& info, const T& value, uint16_t index, const std::string& objectType)
{
    double v = strtod(ValueToString(value).c_str(), NULL);

    struct timespec tp;
	clock_gettime(CLOCK_REALTIME, &tp);
	uint64_t t = ((uint64_t) tp.tv_sec) * 1000LL + (tp.tv_nsec / 1000000);

    org_eclipse_tahu_protobuf_Payload_Metric new_metric;
    memset(&new_metric, 0, sizeof(new_metric));
        
    //new_metric.name = strdup(name);
    new_metric.has_alias = true;
    new_metric.alias = index;
    new_metric.has_timestamp = true;
    new_metric.timestamp = t;
    new_metric.has_datatype = true;
    new_metric.datatype = METRIC_DATA_TYPE_DOUBLE;
    set_metric_value(&new_metric, METRIC_DATA_TYPE_DOUBLE, &v, sizeof(v));
    add_metric_to_payload(this->ddata_payload, &new_metric);
}

bool DNP3::configure()
{
	DNP3::OutStationTCP *outstation = new DNP3::OutStationTCP();
                
	this->setMasterLinkId(1);
	outstation->address = "127.0.0.1";
	outstation->port = 20000;
	outstation->linkId = 10;

	bool enableScan = true;

	this->enableScan(enableScan);
	this->setTimeout(2);
	this->setOutstationScanInterval(5);
	this->setOutstation(outstation);

	return true;
}