#ifndef GAME_CONNECTION_H
#define GAME_CONNECTION_H

#define TIMEOUT 3
#define MAX_CONNECTION_PACKET_SIZE 512

#include <string>

#include <boost\bind.hpp>
#include <boost\asio.hpp>
#include <boost\atomic.hpp>
#include <boost\shared_ptr.hpp>

struct Packet;
struct ABC_Table_Node;
class Character;

class Connection
{
public:
	enum ServerCommand
	{
		LOGIN_OK = 0,
		REGISTRATE_OK = 1,

		HERO_DATA = 10,
		HERO_WORLD = 11,

		CHARACTER_ENTER = 20,
		CHARACTER_MOVE = 21,
		CHARACTER_LEAVE = 29,

		OBJECT_ADD = 30,
		OBJECT_REMOVE = 39,

		GROUP_INVITATION = 40,
		GROUP_ADD = 41,
		GROUP_LEADER = 42,
		GROUP_LEAVE = 43,

		BATTLE_RELOCATE = 50,

		ERROR_LOGIN = 1000,
		ERROR_REGISTRATE = 1001
	};

	enum ClientCommand
	{
		LOGIN = 0,
		LOGIN_RESPONSE = 1,

		REGISTRATE = 2,
		PING = 3,

		HERO_MOVE = 10,
		HERO_STOP = 11,

		GROUP_INVITE = 40,
		GROUP_ACCEPT = 41,
		GROUP_DECLINE = 42,

		BATTLE_JOIN = 50
	};

public:
	std::string name_in_database;

	Connection(boost::asio::io_service& _io_service, const boost::asio::ip::udp::endpoint& _endpoint, const boost::asio::ip::udp::endpoint& _connected_endpoint, ABC_Table_Node* _registry_node);
	~Connection();	

	void Send(Packet* _packet);
	void Send(boost::shared_ptr< Packet > _packet);
	void Send(const int& _command, const std::string& _message);

	inline const boost::asio::ip::udp::endpoint& LocalEndPoint() const { return local_endpoint; };
	inline const boost::asio::ip::udp::socket& Socket() { return socket; };

	void Dispose();

private:
    boost::atomic< bool > disposed;

	ABC_Table_Node* registry_node;

	char* data;
	boost::shared_ptr< Character > character;

    boost::asio::ip::udp::socket socket;

	boost::asio::ip::udp::endpoint local_endpoint;
	boost::asio::ip::udp::endpoint remote_endpoint;
	boost::asio::ip::udp::endpoint connected_endpoint;

	boost::asio::deadline_timer timeout;

	Connection(const Connection& _other);
	Connection& operator=(const Connection* _other);

	std::string ToString(char* _data, unsigned int _size);

	void Receive();
	void HandleReceive(const boost::system::error_code& _error, size_t _received);
	void HandleMessage(size_t _received);

	void HandleSend(const boost::system::error_code& _error, const size_t& _sent);

	void HandleTimeout(const boost::system::error_code& _error);
};

#endif