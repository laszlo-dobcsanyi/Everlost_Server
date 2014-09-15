#include "Core\Macro.h"
#include "Game\Region.h"

#include <boost\foreach.hpp>
#include <boost\shared_ptr.hpp>
#include <boost\lexical_cast.hpp>

#include "Core\Packet.h"
#include "Game\World.h"
#include "Game\Field.h"
#include "Game\Character.h"
#include "Game\Connection.h"
#include "Game\Field_Generated.h"

Region::Region(const int& _x, const int& _y, Field* _field)
	: x(_x),
	  y(_y),
	  field(_field)
{
	#ifdef LOGGING
	Logger::counter_regions++;
	//Logger::Write(7, "\t\t>Creating region @ [" + boost::lexical_cast<std::string>(x) + ":" + boost::lexical_cast<std::string>(y) + "]..");
	#endif // LOGGING

	for(int current = 0; current < 8; ++current)
		neighbours[current] = 0;
}

Region::Region(const int& _x, const int& _y, World* _world)
	: x(_x),
	  y(_y),
	  field(_world->GetField(_x, _y))
{
	#ifdef LOGGING
	Logger::counter_regions++;
	//Logger::Write(7, "\t\t>Creating region @ [" + boost::lexical_cast<std::string>(x) + ":" + boost::lexical_cast<std::string>(y) + "]..");
	#endif // LOGGING

	for(int current = 0; current < 8; ++current)
		neighbours[current] = 0;
}

///

void Region::Relocate(const int& _x, const int& _y)
{
	field->Relocate(_x - x, _y - y);
	x = _x; y = _y;
}

///

Region::~Region()
{
	if (field->Generated()) field->Generated()->Dispose();

	#ifdef LOGGING
	Logger::counter_regions--;
	#endif
}



