#include "Core\Macro.h"
#include "Game\Object.h"

#include <sstream>

#include <boost\lexical_cast.hpp>

Object::Object(const float& _x, const float& _y, const int& _id)
	: x(_x),
	  y(_y),
	  id(_id),
	  id_string(boost::lexical_cast<std::string>(id))
{
	#ifdef LOGGING
	Logger::counter_objects++;
	Logger::Write(LogMask::constructor, LogObject::object, "> Creating Object..");
	#endif

	std::stringstream builder; builder << x << ';' << y << ';' << id;
	data = builder.str();
}

Object::Object(const Object& _other)
	: x(_other.x),
	  y(_other.y),
	  id(_other.id), // TODO GENERATE NEW ID? :O
	  id_string(_other.id_string),
	  data(_other.data)
{
	#ifdef LOGGING
	Logger::counter_objects++;
	Logger::Write(LogMask::constructor, LogObject::object, "> Copying Object..");
	#endif
}

#ifdef LOGGING
Object::~Object()
{
	#ifdef LOGGING
	Logger::Write(LogMask::destructor, LogObject::object, "> Object destructor..");
	Logger::counter_objects--;
	#endif
}
#endif