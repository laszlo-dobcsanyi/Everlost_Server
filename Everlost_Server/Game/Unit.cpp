#include "Core\Macro.h"
#include "Game\Unit.h"

#include <math.h>
#include <sstream>

#include <boost\lexical_cast.hpp>

#include "Game\Region.h"

Unit::Unit(const int _id, const std::string& _name)
	: disposed(false),

	  world(0),
	  region(0),
	  state(0),

	  id(_id),
	  id_string(boost::lexical_cast<std::string>(id)),
	  icon_id(0),
	  name(_name),
	  location(0, 0),
	  move_vector(0, 0),
	  direction(0),
	  moving(false),
	  speed(300)
{
}

const std::string Unit::GetData() const
{
	std::stringstream builder; builder << id << ";" << name << ";" << icon_id << ";" << location.x << ";" << location.y << ";" << direction << ";" << (moving ? speed : 0.0);
	return builder.str();
}

void Unit::Update_Region()
{
	if (region->x < (int)(location.x / 512)) { region = region->neighbours[0]; }
	if ((int)(location.x / 512) < region->x) { region = region->neighbours[4]; }
	if (region->y < (int)(location.y / 512)) { region = region->neighbours[6]; }
	if ((int)(location.y / 512) < region->y) { region = region->neighbours[2]; }
}

void Unit::Rotate(const float& _direction)
{
	move_vector.x = std::cos(_direction);
	move_vector.y = std::sin(_direction);
	direction = _direction;
	moving = true;

	Broadcast_Location();
}

void Unit::Stop()
{
	moving = false;

	Broadcast_Location();
}
