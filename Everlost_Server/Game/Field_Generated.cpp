#include "Core\Macro.h"
#include "Game\Field_Generated.h"

#include <boost\lexical_cast.hpp>

#include "Game\Object.h"
#include "Game\Region.h"
#include "Game\Character.h"

Field_Generated::Field_Generated()
	: Field(),
	  disposed(false)
{
	#ifdef LOGGING
	Logger::counter_field_generateds++;
	Logger::Write(LogMask::constructor, LogObject::field_generated, "> Generated Field constructor..");
	#endif
}

void Field_Generated::Process()
{
	characters.Process_Adding();
	subscribers.Process_Adding(this);

	characters.Process_Removing();
	subscribers.Process_Removing(this);

	Process_Messages();
}

void Field_Generated::ProcessAdding(boost::shared_ptr< Character > _character)
{
	Broadcast_Enter(_character);
}

void Field_Generated::ProcessRemoving(boost::shared_ptr< Character > _character)
{
	Broadcast_Leave(_character);
}

void Field_Generated::Dispose()
{
	if (disposed) return; disposed = true;

	#ifdef LOGGING
	Logger::Write(LogMask::dispose, LogObject::field_generated, "> Disposing Generated Field.");
	#endif

	//Dispose characters?

	delete this;
}

Field_Generated::~Field_Generated()
{
	#ifdef LOGGING
	Logger::counter_field_generateds--;
	Logger::Write(LogMask::destructor, LogObject::field_generated, "> Generated Field Destructor..");
	#endif
	
	BOOST_FOREACH(Object* object, objects)
	{
		delete object;
	}
}
