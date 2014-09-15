#include "Core\Macro.h"
#include "Game\Item.h"

Item::Item()
{
	#ifdef LOGGING
	Logger::Write(LogMask::constructor, LogObject::item, "> Item constructor..");
	#endif
}

Item::~Item()
{
	#ifdef LOGGING
	Logger::Write(LogMask::destructor, LogObject::item, "> Item destructor");
	#endif
}
