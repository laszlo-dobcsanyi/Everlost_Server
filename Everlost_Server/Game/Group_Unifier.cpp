#include "Core\Macro.h"
#include "Game\Group_Unifier.h"

#include "Core\Processor.h"
#include "Game\Group.h"
#include "Game\Connection.h"
#include "Game\Character.h"
#include "Game\Region.h"
#include "Game\Field.h"

extern Processor* updater_service;

Group_Unifier::Group_Unifier(Group* _group, Region* _center, const int& _relocation_direction)
	: disposed(false),

	  strand(updater_service->Service()),

	  group(_group),
	  center(_center),
	  relocation_direction(_relocation_direction),

	  elapsed_time(0.)
{
	#ifdef LOGGING
	Logger::counter_group_unifiers++;
	Logger::Write(LogMask::constructor, LogObject::group_unifier, "> Creating Group Unifier..");
	#endif

	updater_service->Service().post(strand.wrap(boost::bind(&Group_Unifier::Update, this)));
}

void Group_Unifier::Update()
{
	// TODOD RELOCATE GROUP REGIONS AFTER UNIFING!

	boost::chrono::steady_clock::time_point now = boost::chrono::steady_clock::now();
	boost::chrono::duration< float > difference = now - last_update;
	elapsed_time += difference.count();
	last_update = now;

	if (!disposed)
	{
		if (GROUP_UNIFIER_UPDATE_INTERVAL < elapsed_time)
		{
			bool unified = true;

			group->characters.Process_Adding(this);

			//Update leader and check unity
			group->leader->Update(elapsed_time);
			group->leader->region->field->Process();
			if (group->leader->State() & Unit_State::REGION_MOVING) Update_CharacterRegion(group->leader);
			if (!InCenter(group->leader->region->x, group->leader->region->y)) unified = false;

			//Update others in the group and check their unity too
			BOOST_FOREACH(boost::shared_ptr< Character > character, group->characters.data.list)
			{
				character->Update(elapsed_time);
				character->region->field->Process();
				if (character->State() & Unit_State::REGION_MOVING) Update_CharacterRegion(character);
				if (!InCenter(character->region->x, character->region->y)) unified = false;
			}

			group->characters.Process_Removing(this);

			if (unified)
			{
				Unify();

				updater_service->Service().post(strand.wrap(boost::bind(&Group::Update, group)));

				Dispose();
				delete this;
				return;
			}

			elapsed_time = 0.f;
		}

		updater_service->Service().post(strand.wrap(boost::bind(&Group_Unifier::Update, this)));	}
	else
	{
		delete this;
		return;
	}
}

//Delete not used regions in order to start the group again
void Group_Unifier::Unify()
{

}

bool Group_Unifier::InCenter(const int& _x, const int& _y)
{
	if (center->x - VISION_RADIUS <= _x && _x <= center->x + VISION_RADIUS)
		if (center->y - VISION_RADIUS <= _y && _y <= center->y + VISION_RADIUS)
			return true;
	return false;
}

bool Group_Unifier::InBounds(const int& _x, const int& _y)
{
	if (VISION_RADIUS <= _x && _x <= 8 * VISION_RADIUS)
		if (VISION_RADIUS <= _y && _y <= 3 * VISION_RADIUS)
			return true;
	return false;
}

void Group_Unifier::Update_CharacterRegion(boost::shared_ptr< Character > _character)
{
	//Add and remove subscribes to regions
	int direction = StateToDirection(_character->State());

	if (!InBounds(_character->region->neighbours[direction]->x, _character->region->neighbours[direction]->y)) return;

	Update_CharacterSubscription(_character, direction);

	//Remove from current field and check if we need to process field, to send all packets
	_character->region->field->characters.Remove(_character);
	if (_character->region->field->Generated()) _character->region->field->characters.Process_Removing();


	//Update region and add to field
	_character->region = _character->region->neighbours[direction];
	_character->region->field->characters.Add(_character);
}

//When a character moves from a region, this function removes the character from the subscribers of the left field, and adds it to the entered fields 
void Group_Unifier::Update_CharacterSubscription(boost::shared_ptr< Character > _character, const int& _direction)
{
	if (_direction % 2)
	{
		Region* added = _character->region->neighbours[neighbour_direction[_direction - 1]]; for(int distance = 0; distance < VISION_RADIUS; ++distance) added = added->neighbours[neighbour_direction[_direction - 1]];
		Region* removed = _character->region->neighbours[deleted_direction[_direction - 1]]; for(int distance = 1; distance < VISION_RADIUS; ++distance) removed = removed->neighbours[deleted_direction[_direction - 1]];

		for (int current = 0; current < 2 * VISION_RADIUS + 1; ++current)
		{
			added->field->subscribers.Add(_character); added->field->Process();
			removed->field->subscribers.Remove(_character); removed->field->Process();

			added = added->neighbours[next[_direction - 1]];
			removed = removed->neighbours[next[_direction - 1]];
		}

		added = _character->region->neighbours[neighbour_direction[mod(_direction + 1, 8)]]; for(int distance = 1; distance < VISION_RADIUS; ++distance) added = added->neighbours[neighbour_direction[mod(_direction + 1, 8)]];
		removed = _character->region->neighbours[deleted_direction[mod(_direction + 1, 8)]]; for(int distance = 1; distance < VISION_RADIUS; ++distance) removed = removed->neighbours[deleted_direction[mod(_direction + 1, 8)]];

		added = added->neighbours[_direction];
		removed = removed->neighbours[_direction - 1];

		for (int current = 0; current < 2 * VISION_RADIUS; ++current)
		{
			added->field->subscribers.Add(_character); added->field->Process();
			removed->field->subscribers.Remove(_character); removed->field->Process();

			added = added->neighbours[next[mod(_direction + 1, 8)]];
			removed = removed->neighbours[next[mod(_direction + 1, 8)]];
		}
	}
	else
	{
		Region* added = _character->region->neighbours[neighbour_direction[_direction]]; for(int distance = 1; distance < VISION_RADIUS; ++distance) added = added->neighbours[neighbour_direction[_direction]];
		Region* removed = _character->region->neighbours[deleted_direction[_direction]]; for(int distance = 1; distance < VISION_RADIUS; ++distance) removed = removed->neighbours[deleted_direction[_direction]];

		added = added->neighbours[_direction];

		for (int current = 0; current < 2 * VISION_RADIUS + 1; ++current)
		{
			added->field->subscribers.Add(_character); added->field->Process();
			removed->field->subscribers.Remove(_character); removed->field->Process();

			added = added->neighbours[next[_direction]];
			removed = removed->neighbours[next[_direction]];
		}
	}
}


void Group_Unifier::ProcessAdding(boost::shared_ptr< Character > _character)
{
	// TODO
}

void Group_Unifier::ProcessRemoving(boost::shared_ptr< Character > _character)
{
	Region* base = _character->region; for (int current = 0; current < VISION_RADIUS; ++current) base = base->neighbours[3];
	for(int row = 0; row < VISION_RADIUS * 2 + 1; ++row)
	{
		Region* current = base;
		for(int column = 0; column < VISION_RADIUS * 2 + 1; ++column)
		{
			//Remove from subscribers
			current->field->subscribers.Remove(_character);

			//Check if we need to process the field
			if (current->field->Generated() && current->field->characters.data.number == 0) current->field->Process();

			current = current->neighbours[0];
		}

		base = base->neighbours[6];
	}

	//Remove character from it's field
	_character->region->field->characters.Remove(_character);
	
	//Check if we need to process the Field, to remove character
	if ((_character->region->field->Generated()) && (_character->region->field->characters.data.number == 1)) _character->region->field->Process();

	//Update leader if we need to
	if (_character == _character->group->leader)
	{
		if (0 < _character->group->characters.data.number)
		{
			_character->group->leader = _character->group->characters.data.list.front();
			_character->group->characters.data.list.pop_front();
			--_character->group->characters.data.number;

			//Send leaver and leader data
			_character->connection->Send(Connection::GROUP_LEAVE, _character->ID_String());

			_character->group->leader->connection->Send(Connection::GROUP_LEAVE, _character->ID_String());
			_character->group->leader->connection->Send(Connection::GROUP_LEADER, _character->group->leader->ID_String());

			BOOST_FOREACH(boost::shared_ptr< Character > character, _character->group->characters.data.list)
			{
				character->connection->Send(Connection::GROUP_LEAVE, _character->ID_String());
				character->connection->Send(Connection::GROUP_LEADER, _character->group->leader->ID_String());
			}
		}
		else
		{
			_character->group->State(_character->group->State() | Group_State::EMPTY);
			_character->group->Dispose();
			Dispose();
		}
	}
	else
	{
		//Send leave data
		_character->group->leader->connection->Send(Connection::GROUP_LEAVE, _character->ID_String());

		BOOST_FOREACH(boost::shared_ptr< Character > character, _character->group->characters.data.list)
		{
			character->connection->Send(Connection::GROUP_LEAVE, _character->ID_String());
		}
	}

	//Check if we need to create a new group for the character
	if (_character->State() & Unit_State::GROUP_CREATE)
	{
		//Remove flag from state
		_character->State(_character->State() & (~Unit_State::GROUP_CREATE));

		_character->group = new Group(_character);
	}
	else
	{
		//Check if we need to add character to the existing inviter group
		if (_character->State() & Unit_State::GROUP_JOIN)
		{
			//Remove flag from state
			_character->State(_character->State() & (~Unit_State::GROUP_JOIN));

			_character->group = _character->inviter;
			_character->region = _character->inviter->leader->region;
			_character->inviter->characters.Add(_character);
			_character->inviter->invited.reset();
			_character->inviter = 0;
		}
	}
}


void Group_Unifier::Dispose()
{
	if (disposed) return; disposed = true;
	#ifdef LOGGING
	Logger::Write(LogMask::dispose, LogObject::group_unifier, "> Disposing Group Unifier..");
	#endif
}

Group_Unifier::~Group_Unifier()
{
	#ifdef LOGGING
	Logger::Write(LogMask::destructor, LogObject::group_unifier, "> Group Unifier destructor..");
	Logger::counter_group_unifiers--;
	#endif
}