#include "Core\Macro.h"
#include "Game\Connection.h"

#undef ERROR

#include <boost\lexical_cast.hpp>

#include "Core\Packet.h"
#include "Core\Registry.h"
#include "Game\Character.h"

Connection::Connection(boost::asio::io_service& _io_service, const boost::asio::ip::udp::endpoint& _endpoint, const boost::asio::ip::udp::endpoint& _connected_endpoint, ABC_Table_Node* _registry_node)
	: disposed(false),
	  registry_node(_registry_node),

	  data(new char[MAX_CONNECTION_PACKET_SIZE]),

	  socket(_io_service, _endpoint),

	  remote_endpoint(_connected_endpoint),
	  local_endpoint(_endpoint),

	  timeout(_io_service, boost::posix_time::seconds(TIMEOUT))
{
	#ifdef LOGGING
	Logger::counter_connections++;
	Logger::Write(LogMask::constructor, LogObject::connection, "> Connection constructor @ [" + boost::lexical_cast<std::string>(local_endpoint.address()) + ":" +  boost::lexical_cast<std::string>(local_endpoint.port()) + "] for [" +
		boost::lexical_cast<std::string>(remote_endpoint.address()) + ":" + boost::lexical_cast<std::string>(remote_endpoint.port()) + "]!");
	#endif

	DWORD size = 64 * 1024;
	setsockopt(socket.native(), SOL_SOCKET, SO_SNDBUF, (char *)&size, sizeof(WORD));
	setsockopt(socket.native(), SOL_SOCKET, SO_RCVBUF, (char *)&size, sizeof(WORD));

	timeout.expires_from_now(boost::posix_time::seconds(TIMEOUT));
	timeout.async_wait(boost::bind(&Connection::HandleTimeout, this, boost::asio::placeholders::error));

	Receive();
}

void Connection::Receive()
{
	socket.async_receive_from(boost::asio::buffer(data, MAX_CONNECTION_PACKET_SIZE), connected_endpoint,
		boost::bind(&Connection::HandleReceive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void Connection::HandleReceive(const boost::system::error_code& _error, size_t _received)
{
	if ((!_error) && (0 < _received))
	{
		if (connected_endpoint == remote_endpoint)
		{
			HandleMessage(_received);

			timeout.expires_from_now(boost::posix_time::seconds(TIMEOUT));
			timeout.async_wait(boost::bind(&Connection::HandleTimeout, this, boost::asio::placeholders::error));
		}
		else
		{
			#ifdef LOGGING
			Logger::Write(LogMask::error, LogObject::connection, "# Corrupted Message received from [" + boost::lexical_cast<std::string>(connected_endpoint.address()) + ":" + boost::lexical_cast<std::string>(connected_endpoint.port()) + "]!");
			#endif
		}

		Receive();
	}
	else
	{
		#ifdef LOGGING
		Logger::Write(LogMask::error, LogObject::connection, "# Error @ Connection::HandleReceive " + _error.message() + " (" + boost::lexical_cast<std::string>(_received) + ") from [" + boost::lexical_cast<std::string>(connected_endpoint.address()) + ":" + boost::lexical_cast<std::string>(connected_endpoint.port()) + "]!");
		#endif

		Dispose();
	}
}

void Connection::Send(Packet* _packet)
{
	socket.async_send_to(boost::asio::buffer(_packet->data, _packet->size), remote_endpoint,
		boost::bind(&Connection::HandleSend, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void Connection::Send(boost::shared_ptr< Packet> _packet)
{
	//#ifdef LOGGING
	//Logger::Write(LogMask::message, LogObject::connection, "> Connection sending packet..");
	//#endif

	socket.async_send_to(boost::asio::buffer(_packet->data, _packet->size), remote_endpoint,
		boost::bind(&Connection::HandleSend, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void Connection::Send(const int& _command, const std::string& _message)
{
	//#ifdef LOGGING
	//Logger::Write(LogMask::message, LogObject::connection, "\t> Connection sending created packet..");
	//#endif

	Packet packet(_command, _message);
	socket.async_send_to(boost::asio::buffer(packet.data, packet.size), remote_endpoint,
		boost::bind(&Connection::HandleSend, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void Connection::HandleSend(const boost::system::error_code& _error, const size_t& _sent)
{
	if (_error)
	{
		#ifdef LOGGING
		Logger::Write(LogMask::error, LogObject::connection, "# Error @ Connection:HandleSend " + _error.message());
		#endif
	}
}

void Connection::HandleTimeout(const boost::system::error_code& _error)
{
	if (!_error) Dispose();
}

void Connection::Dispose()
{
	if (disposed) return; disposed = true;

	#ifdef LOGGING
	Logger::Write(LogMask::dispose, LogObject::connection, "> Disposing Connection" + boost::lexical_cast<std::string>(local_endpoint.port()) + "..");
	#endif
		
	socket.close();

	if (character)
	{
		character->Dispose();
		character.reset();

	}
	else delete this;
}

Connection::~Connection()
{	
	#ifdef LOGGING
	Logger::Write(LogMask::destructor, LogObject::connection, "> Connection" + boost::lexical_cast<std::string>(local_endpoint.port()) + " destructor..");
	#endif

	delete[] data;

	registry_node->connection = 0;

	#ifdef LOGGING
	Logger::counter_connections--;
	#endif
}

