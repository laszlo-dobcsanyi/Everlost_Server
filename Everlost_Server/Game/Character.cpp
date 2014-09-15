#include "Core\Macro.h"
#include "Game\Character.h"

#include <fstream>

#include <boost\lexical_cast.hpp>

#include "Core\Realm.h"
#include "Game\Group.h"
#include "Game\World.h"
#include "Game\Field.h"
#include "Game\Region.h"
#include "Core\Packet.h"
#include "Game\Connection.h"

extern Realm* realm;

/// PUBLIC

Character::Character(const std::string& _name, Connection* _connection)
	: Unit(realm->characters_generator.Next(), _name),
	  connection(_connection),
	  
	  group(0),
	  battle(0),
	  inviter(0)
{
	#ifdef LOGGING
	Logger::counter_characters++;
	Logger::Write(LogMask::constructor, LogObject::character, "> Character" + boost::lexical_cast<std::string>(id) + " constructor..");
	#endif

	int spawn_world = 0;

	std::ifstream data("characters\\" + _name + ".data");
	data >> icon_id >> spawn_world >> location.x >> location.y;
	data.close();

	world = realm->GetWorld(spawn_world);

	std::stringstream builder; builder << id << ";" << name << ";" << icon_id << ";" << location.x << ";" << location.y << ";" << direction << ";" << (moving ? speed : 0.0);
	connection->Send(Connection::HERO_DATA, builder.str());

	builder.str(""); builder.clear(); builder << world->id;
	connection->Send(Connection::HERO_WORLD, builder.str());
}

uint8_t Character::Update(const float& _elapsedtime)
{
	//Reset region moving mask
	state &= 0xF0;

	if (moving)
	{
		location.x += (move_vector.x * speed * _elapsedtime);
		location.y += (move_vector.y * speed * _elapsedtime);

		if (region->x < (int)(location.x / 512)) { state |= Unit_State::REGION_MOVING_RIGHT; }
		if ((int)(location.x / 512) < region->x) { state |= Unit_State::REGION_MOVING_LEFT; }
		if (region->y < (int)(location.y / 512)) { state |= Unit_State::REGION_MOVING_DOWN; }
		if ((int)(location.y / 512) < region->y) { state |= Unit_State::REGION_MOVING_UP; }
	}
	return state;
}

void Character::Teleport(const Vector2& _location)
{
	location = _location;
	Broadcast_Location();
}

void Character::Dispose()
{
	if (disposed) return; disposed = true;

	#ifdef LOGGING
	Logger::Write(LogMask::dispose, LogObject::character, "> Disposing Character..");
	#endif

	group->characters.Remove(shared_from_this());

	if (inviter) inviter->Invitation_Decline();
}

Character::~Character()
{
	#ifdef LOGGING
	Logger::Write(LogMask::destructor, LogObject::character, "> Character" + boost::lexical_cast<std::string>(id) + " destructor..");
	#endif

	delete connection;

	#ifdef LOGGING
	Logger::counter_characters--;
	#endif
}

///

void Character::Broadcast_Location()
{
	#ifdef LOGGING
	Logger::Write(LogMask::message, LogObject::character, "> Broadcasting Location..");
	#endif

	std::stringstream builder; builder << id << ";" << location.x << ";" << location.y << ";" << direction << ";" << (moving ? speed : 0.0f);

	region->field->Broadcast(boost::shared_ptr<PacketPair>(new PacketPair(new Packet(Connection::CHARACTER_MOVE, builder.str()))));
}