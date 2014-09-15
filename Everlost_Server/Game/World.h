#ifndef GAME_WORLD_H
#define GAME_WORLD_H

#include <string>

#include "Game\Region.h"

class Field;
class Field_Loaded;

class World
{
public:
	int id;

	World(const int& _id);
	inline ~World() { };

	Field* GetField(const int& _x, const int& _y) const;

private:
	std::string name;

	int width, height;

	Field_Loaded** fields;

	World(const World& _other);
	World& operator=(const World* _other);
};

#endif