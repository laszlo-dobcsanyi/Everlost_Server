#include "Core\Macro.h"
#include "Core\Packet.h"

Packet::Packet(const int& _command, const std::string& _message)
	: data(new char[4 + _message.size() + 1]),
	  size(4 + _message.size() + 1)
{
	#ifdef LOGGING
	Logger::counter_packets++;
	#endif

	data[0] = (_command >> 24) & 0xFF;
	data[1] = (_command >> 16) & 0xFF;
	data[2] = (_command >> 8) & 0xFF;
	data[3] = _command & 0xFF;
	memcpy(data + 4, _message.c_str(), _message.size() + 1);
}

Packet::~Packet()
{
	delete[] data;

	#ifdef LOGGING
	Logger::counter_packets--;
	#endif
};

PacketPair::PacketPair(Packet* _packet)
	: sender(0), packet(_packet)
{
	#ifdef LOGGING
	Logger::counter_packet_pairs++;
	#endif
};

PacketPair::PacketPair(boost::shared_ptr< Unit > _sender, Packet* _packet)
	: sender(_sender), packet(_packet)
{	
	#ifdef LOGGING
	Logger::counter_packet_pairs++;
	#endif
};

PacketPair::~PacketPair()
{ 
	delete packet;
	
	#ifdef LOGGING
	Logger::counter_packet_pairs--;
	#endif
};