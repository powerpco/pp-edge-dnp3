#include <iostream>
#include <vector>

#include <opendnp3/ConsoleLogger.h>
#include <opendnp3/DNP3Manager.h>
#include <opendnp3/channel/PrintingChannelListener.h>
#include <opendnp3/logging/LogLevels.h>
#include <opendnp3/master/DefaultMasterApplication.h>
#include <opendnp3/master/PrintingCommandResultCallback.h>
#include <opendnp3/master/PrintingSOEHandler.h>
#include <opendnp3/outstation/IUpdateHandler.h>
#include <opendnp3/outstation/SimpleCommandHandler.h>
#include <opendnp3/master/ISOEHandler.h>

#include <tahu.h>
#include <tahu.pb.h>
#include <pb_decode.h>
#include <pb_encode.h>

#include "mqtt_client.h"
#include "settings.h"

#define DEFAULT_APPLICATION_TIMEOUT "5" // seconds
#define DEFAULT_MASTER_LINK_ID "1"
#define DEFAULT_TCP_ADDR "127.0.0.1"
#define DEFAULT_TCP_PORT "20000"   
#define DEFAULT_OUTSTATION_ID "10"
#define DEFAULT_OUTSTATION_SCAN_INTERVAL "30" // seconds

using namespace opendnp3;
class DNP3 {
    public:
        class OutStationTCP
		{
			public:
				OutStationTCP()
				{
					// Set default values
					address = DEFAULT_TCP_ADDR;
					port = (short unsigned int)atoi(DEFAULT_TCP_PORT);
					linkId = (uint16_t)atoi(DEFAULT_OUTSTATION_ID);
				};
				std::string	address;
				short unsigned int port;
				uint16_t linkId;
		};
    public:
        DNP3()
		{
			m_manager = NULL; // configure() creates the object
			m_enableScan = false; // Scan outstation (Integrity Poll)
			// Default scan interval in seconds
			m_outstationScanInterval = (unsigned long)atol(DEFAULT_OUTSTATION_SCAN_INTERVAL);
			// Network timeout default
			m_applicationTimeout = (unsigned long)DEFAULT_APPLICATION_TIMEOUT;
		};
        ~DNP3()
		{
			if (m_manager)
			{
				delete m_manager;
			}
		};
        void setMasterLinkId(uint16_t id)
		{
			m_masterId = id;
		};
		uint16_t getMasterLinkId() { return m_masterId; };
        unsigned long getTimeout() const { return m_applicationTimeout; };
		void setTimeout(unsigned long val)
		{
			m_applicationTimeout = val;
		};

		bool start();
        bool configure(settings* s);
		// Stop master anc close outstation connection
		void stop()
		{
			if (m_manager)
			{
				m_manager->Shutdown();
				delete m_manager;
				m_manager = NULL;
			}
		};

        void enableScan(bool val) { m_enableScan = val; };
		bool isScanEnabled() const { return m_enableScan; };
		unsigned long getOutstationScanInterval() const
		{
			return m_outstationScanInterval;
		};
		void setOutstationScanInterval(unsigned long val)
		{
			m_outstationScanInterval = val;
		};
		DNP3::OutStationTCP* getOutstation() const
		{
			return m_outstation;
		};
		void setOutstation(DNP3::OutStationTCP* outstation)
		{
			m_outstation = outstation;
		};
		unsigned long getOffset() const
		{
			return m_offset;
		}
		void setOffset(unsigned long offset)
		{
			m_offset = offset;
		}

		void send(uint8_t* binary_buffer, size_t message_length) 
		{
			m_client.publish(binary_buffer, message_length);
		};

		void setGroup(string group)
        {
            m_group = group;
        }
        string getGroup() { return m_group; };

        void setEdgeNode(string edgeNode)
        {
            m_edge_node = edgeNode;
        }
        string getEdgeNode() { return m_edge_node; };

        void setDevice(string device)
        {
            m_device = device;
        }
        string getDevice() { return m_device; };
    private:
        uint16_t m_masterId;
		DNP3Manager* m_manager;
		bool m_enableScan;
		unsigned long m_outstationScanInterval;
		unsigned long m_applicationTimeout;
        DNP3::OutStationTCP* m_outstation;
		unsigned long m_offset;
		mqtt_client m_client;
		string m_group;
        string m_edge_node;
        string m_device;
};

class DNP3SOEHandler : public opendnp3::ISOEHandler
{
	public:
		DNP3SOEHandler(DNP3* dnp3, std::string& name)
		{
			m_dnp3 = dnp3;
			m_label = name;
		};

		// Data callbacks
		// We get data from these objects only 
		void Process(const HeaderInfo& info, const ICollection<Indexed<Counter>>& values) override
        {
            return this->DNP3DataCallback(info, values, "Counter");
        };

        void Process(const HeaderInfo& info, const ICollection<Indexed<Binary>>& values) override
        {
            return this->DNP3DataCallback(info, values, "Binary");
        };
		
        void Process(const HeaderInfo& info, const ICollection<Indexed<BinaryOutputStatus>>& values) override
        {
            return this->DNP3DataCallback(info, values, "BinaryOutputStatus");
        };

        void Process(const HeaderInfo& info, const ICollection<Indexed<Analog>>& values) override
        {
            return this->DNP3DataCallback(info, values, "Analog");
        };

        // We don't get data from these
        void Process(const HeaderInfo& info, const ICollection<Indexed<DoubleBitBinary>>& values) override {};
        void Process(const HeaderInfo& info, const ICollection<Indexed<FrozenCounter>>& values) override {};
        void Process(const HeaderInfo& info, const ICollection<Indexed<AnalogOutputStatus>>& values) override {};
        void Process(const HeaderInfo& info, const ICollection<Indexed<OctetString>>& values) override {};
        void Process(const HeaderInfo& info, const ICollection<Indexed<TimeAndInterval>>& values) override {};
        void Process(const HeaderInfo& info, const ICollection<Indexed<BinaryCommandEvent>>& values) override {};
        void Process(const HeaderInfo& info, const ICollection<Indexed<AnalogCommandEvent>>& values) override {};
        void Process(const HeaderInfo& info, const ICollection<DNPTime>& values) override {};
		void BeginFragment(const ResponseInfo& info){};
    	void EndFragment(const ResponseInfo& info){};

    protected:
        // Callback for data receiving:
        // solicited and unsolicited messages
		template<class T> void DNP3DataCallback(const HeaderInfo& info, const ICollection<Indexed<T>>& values, const std::string& objectType);

        // Process a data element from callback
        // and ingest data into Fledge
        template<class T> void dataElement(const opendnp3::HeaderInfo& info, const T& value, uint16_t index, const std::string& objectType);
    
    private:
        DNP3* m_dnp3;
		std::string	m_label;
		org_eclipse_tahu_protobuf_Payload* ddata_payload;
};

template<class T> std::string ValueToString(const T& meas)
{
	return std::to_string(meas.value);
}