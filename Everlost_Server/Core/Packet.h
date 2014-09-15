#ifndef CORE_PACKET_H
#define CORE_PACKET_H

#include <string>

#include <boost\shared_ptr.hpp>

struct Packet
{
public:
	int size;
	char* data;

	Packet(const int& _command, const std::string& _message);
	~Packet();

private:
	Packet(const Packet& _other);
	Packet& operator=(const Packet* other);
};

class Unit;

struct PacketPair
{
public:
	boost::shared_ptr< Unit > sender;
	Packet* packet;

	PacketPair(Packet* _packet);
	PacketPair(boost::shared_ptr< Unit> _sender, Packet* _packet);
	~PacketPair();

private:
	PacketPair(const PacketPair& _other);
	PacketPair& operator=(const PacketPair& _other);
};

#endif
