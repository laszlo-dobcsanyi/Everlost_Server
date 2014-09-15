#include "Core\Macro.h"
#include "Core\Gateway.h"

#include <boost\bind.hpp>
#include <boost\lexical_cast.hpp>
#include <boost\algorithm\string.hpp>

#include "Core\Packet.h"
#include "Core\Registry.h"
#include "Core\Processor.h"
#include "Game\Connection.h"

extern Registry* registry;
extern Processor* network_service;

/// PUBLIC

Gateway::Gateway(const boost::asio::ip::udp::endpoint& _endpoint)
	: data(new char[MAX_GATEWAY_PACKET_SIZE]),
	  port_generator(CHARACTERS_NUMBER),

	  gateway(network_service->Service(), _endpoint),
	  thread(boost::bind(&Gateway::Listen, this))
{
	#ifdef LOGGING
	Logger::Write(LogMask::initialize, LogObject::gateway, "> Gateway @ [" + boost::lexical_cast<std::string>(_endpoint.address()) + ":" + boost::lexical_cast<std::string>(_endpoint.port()) + "] created!");
	#endif
}

/// PRIVATE

void Gateway::Listen()
{
	gateway.async_receive_from(boost::asio::buffer(data, MAX_GATEWAY_PACKET_SIZE), connected_endpoint,
		boost::bind(&Gateway::HandleReceive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

std::string Gateway::ToString(char* _data, unsigned int _size)
{
	std::string value = "";
	for(unsigned int current = 0; current < _size; ++current) value += _data[current];
	return value;
}

void Gateway::HandleReceive(const boost::system::error_code& _error, const size_t& _received)
{
	if ((!_error) && (0 < _received))
	{
		int command = 0x00000000; command |= data[0] << 24; command |= data[1] << 16; command |= data[2] << 8; command |= data[3];
		std::vector<std::string> arguments; boost::split(arguments, ToString(data + 4, _received - 4), boost::is_any_of(";"));

		if (command == Connection::ClientCommand::LOGIN)
		{
			ABC_Table_Node* node = registry->GetNode(arguments[0]);
			if (node)
			{
				// TODO cryptograhy?
				if(node->pass == arguments[1].data())
				{
					if (!node->connection)
					{
						node->connection = new Connection(network_service->Service(), boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(GATEWAY_ADDRESS), 40000 + port_generator.Next()), connected_endpoint, node);

						#ifdef LOGGING
						Logger::Write(LogMask::message, LogObject::gateway, "> Gateway sending packet..");
						#endif
				
						Packet packet(Connection::ServerCommand::LOGIN_OK, boost::lexical_cast<std::string>(node->connection->LocalEndPoint().port()));
						gateway.async_send_to(boost::asio::buffer(packet.data, packet.size), connected_endpoint,
							boost::bind(&Gateway::HandleSend, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
					}
					else
					{
						#ifdef LOGGING
						Logger::Write(LogMask::error, LogObject::gateway, "# Already logged in!");
						#endif

						Packet packet(Connection::ServerCommand::ERROR_LOGIN, "already");
						gateway.async_send_to(boost::asio::buffer(packet.data, packet.size), connected_endpoint,
							boost::bind(&Gateway::HandleSend, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
					}
				}
				else
				{
					#ifdef LOGGING
					Logger::Write(LogMask::error, LogObject::gateway, "# Wrong password!");
					#endif

					Packet packet(Connection::ServerCommand::ERROR_LOGIN, "pass");
					gateway.async_send_to(boost::asio::buffer(packet.data, packet.size), connected_endpoint,
						boost::bind(&Gateway::HandleSend, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
				}
			}
			else
			{
				#ifdef LOGGING
				Logger::Write(LogMask::error, LogObject::gateway, "# Username not found!");
				#endif

				Packet packet(Connection::ServerCommand::ERROR_LOGIN, "user");
				gateway.async_send_to(boost::asio::buffer(packet.data, packet.size), connected_endpoint,
					boost::bind(&Gateway::HandleSend, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
			}
		}

		if (command == Connection::ClientCommand::REGISTRATE)
		{
			// TODO Check input data (length, content)!
			if(registry->AddNode(arguments[0], arguments[1]))
			{
				Packet packet(Connection::ServerCommand::REGISTRATE_OK, "");
				gateway.async_send_to(boost::asio::buffer(packet.data, packet.size), connected_endpoint,
					boost::bind(&Gateway::HandleSend, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
			}
			else
			{
				Packet packet(Connection::ServerCommand::ERROR_REGISTRATE, "");
				gateway.async_send_to(boost::asio::buffer(packet.data, packet.size), connected_endpoint,
					boost::bind(&Gateway::HandleSend, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
			}
		}
	}
	else
	{
		#ifdef LOGGING
		Logger::Write(LogMask::error, LogObject::gateway, "# Error @ Gateway::HandleReceive " + _error.message() + " (" + boost::lexical_cast<std::string>(_received) + ") from [" +
			boost::lexical_cast<std::string>(connected_endpoint.address()) + ":" + boost::lexical_cast<std::string>(connected_endpoint.port()) + "]!\n");
		#endif
	}

	Listen();
}

void Gateway::HandleSend(const boost::system::error_code& _error, const size_t& _sent)
{
	if (_error)
	{
		#ifdef LOGGING
		Logger::Write(LogMask::error, LogObject::gateway, "# Error @ Gateway:HandleSend " + _error.message());
		#endif
	}
}