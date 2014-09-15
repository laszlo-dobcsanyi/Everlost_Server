#ifndef GAME_CHARACTER_H
#define GAME_CHARACTER_H

#include <boost\enable_shared_from_this.hpp>

#include "Game\Unit.h"

class Field;
class Group;
class Battle;
class Connection;

class Character
	: public Unit,
	  public boost::enable_shared_from_this< Character >
{
public:
	Group* group;
	Group* inviter;
	Battle* battle;
	Connection* connection;

	Character(const std::string& _name, Connection* _connection);
	~Character();

	uint8_t Update(const float& _elapsedtime);
	void Dispose();

	void Teleport(const Vector2& _location);
	void Broadcast_Location();

private:
	Character(const Character& _other);
	Character& operator=(const Character* _other);
};

#endif