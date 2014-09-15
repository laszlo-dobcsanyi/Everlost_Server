#ifndef CORE_GATEWAY_H
#define CORE_GATEWAY_H

#define GATEWAY_ADDRESS "127.0.0.2"
#define MAX_GATEWAY_PACKET_SIZE 512

#include <boost\asio.hpp>
#include <boost\thread.hpp>

#include "Core\Generator.h"

class Gateway
{
public:
	Gateway(const boost::asio::ip::udp::endpoint& _endpoint);

private:
	char* data;
	Generator port_generator;

	boost::asio::ip::udp::socket gateway;
	boost::thread thread;

	boost::asio::ip::udp::endpoint connected_endpoint;

	Gateway(const Gateway& _other);
	Gateway& operator=(const Gateway* _other);

	std::string ToString(char* _data, unsigned int _size);

	void Listen();
	void HandleReceive(const boost::system::error_code& _error, const size_t& _received);
	void HandleSend(const boost::system::error_code& _error, const size_t& _sent);
};

#endif