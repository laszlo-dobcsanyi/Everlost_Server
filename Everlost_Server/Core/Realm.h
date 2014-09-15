#ifndef CORE_REALM_H
#define CORE_REALM_H

#include "Core\Generator.h"

class World;

class Realm
{
public:
	Generator characters_generator;

	Realm();
	inline ~Realm();

	inline World* GetWorld(const int& _id) const { switch(_id) { case 0: return world0; case 1: return world1; default: return world0; } };

private:
	World* world0;
	World* world1;

	Realm(const Realm& _other);
	Realm& operator=(const Realm* _other);
};

#endif