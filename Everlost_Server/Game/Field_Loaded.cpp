#include "Core\Macro.h"
#include "Game\Field_Loaded.h"

#include <boost\bind.hpp>

#include "Game\Character.h"

Field_Loaded::Field_Loaded()
	: Field(),
	  thread(boost::bind(&Field_Loaded::Update, this))
{
}

void Field_Loaded::Update()
{
	do
    {
		Process();

		boost::this_thread::sleep(boost::posix_time::milliseconds(FIELD_UPDATE_INTERVAL));
	} while (true);
}

void Field_Loaded::Process()
{
	characters.Process_Adding();
	subscribers.Process_Adding(this);

	characters.Process_Removing();
	subscribers.Process_Removing(this);

	Process_Messages();
}

void Field_Loaded::ProcessAdding(boost::shared_ptr< Character > _character)
{
	Broadcast_Enter(_character);
}

void Field_Loaded::ProcessRemoving(boost::shared_ptr< Character > _character)
{
	Broadcast_Leave(_character);
}