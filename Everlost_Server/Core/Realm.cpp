#include "Core\Macro.h"
#include "Core\Realm.h"

#include "Game\World.h"

/// PUBLIC

Realm::Realm()
	: characters_generator(CHARACTERS_NUMBER),
	  world0(new World(0)),
	  world1(new World(1))
{
	#ifdef LOGGING
	Logger::Write(LogMask::initialize, LogObject::realm, "> Realm created!");
	#endif
}
