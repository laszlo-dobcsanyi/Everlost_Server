#include "Core\Macro.h"
#include "Game\Field.h"

#include <boost\foreach.hpp>
#include <boost\lexical_cast.hpp>

#include "Core\Packet.h"
#include "Game\Object.h"
#include "Game\Character.h"
#include "Game\Connection.h"

Field::Field()
{

}

void Field::Relocate(const int& _offset_x, const int& _offset_y)
{
	float delta_x = _offset_x * 512.f;
	float delta_y = _offset_y * 512.f;

	BOOST_FOREACH(Object* object, objects)
	{
		object->X(object->X() + delta_x);
		object->Y(object->Y() + delta_y);
	}

	BOOST_FOREACH(boost::shared_ptr< Character> character, characters.data.list)
	{
		character->Location(Vector2(character->Location().x + delta_x, character->Location().y + delta_y));
	}
}


void Field::Process_Messages()
{
	//Process sends
	boost::unique_lock<boost::shared_mutex> sends_lock(sends.mutex);

	BOOST_FOREACH(boost::shared_ptr< PacketPair > packet_pair, sends.list)
	{
		BOOST_FOREACH(boost::shared_ptr< Character > character, characters.data.list)
		{
			if (packet_pair->sender != character) character->connection->Send(packet_pair->packet);
		}
	}
	sends.list.clear();
	sends.number = 0;

	sends_lock.unlock();
	
	//Process broadcasts
	boost::unique_lock<boost::shared_mutex> broadcast_lock(broadcasts.mutex);

	BOOST_FOREACH(boost::shared_ptr< PacketPair > packet_pair, broadcasts.list)
	{
		BOOST_FOREACH(boost::shared_ptr< Character > subscriber, subscribers.data.list)
		{
			if (packet_pair->sender != subscriber) subscriber->connection->Send(packet_pair->packet);
		}
	}
	broadcasts.list.clear();
	broadcasts.number = 0;

	broadcast_lock.unlock();
}

void Field::Broadcast_Enter(boost::shared_ptr< Character > _character)
{
	BOOST_FOREACH(boost::shared_ptr< Character > character, characters.data.list)
	{
		if (_character != character)
		{
			_character->connection->Send(Connection::CHARACTER_ENTER, character->GetData());
			character->connection->Send(Connection::CHARACTER_ENTER, _character->GetData());
		}
	}

	BOOST_FOREACH(Object* object, objects)
	{
		_character->connection->Send(Connection::OBJECT_ADD, object->GetData());
	}
}

void Field::Broadcast_Leave(boost::shared_ptr< Character > _character)
{
	//Send(boost::shared_ptr< PacketPair >(new PacketPair(_character, new Packet(Connection::CHARACTER_LEAVE, _character->ID_String()))));
	
	BOOST_FOREACH(boost::shared_ptr< Character > character, characters.data.list)
	{
		if (_character != character)
		{
			_character->connection->Send(Connection::CHARACTER_LEAVE, character->ID_String());
			character->connection->Send(Connection::CHARACTER_LEAVE, _character->ID_String());
		}
	}

	BOOST_FOREACH(Object* object, objects)
	{
		_character->connection->Send(Connection::OBJECT_REMOVE, object->ID_String());
	}
}