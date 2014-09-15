#include "Core\Macro.h" 
#include "Game\Battle.h"

#include "Core\Packet.h"
#include "Core\Processor.h"
#include "Game\Connection.h"
#include "Game\Group.h"
#include "Game\Character.h"
#include "Game\Field.h"
#include "Game\Region.h"
#include "Game\Group_Unifier.h"

extern Processor* updater_service;

Battle::Battle(Group* _group1, Group* _group2)
	: disposed(false),

	  strand(updater_service->Service()),
	  
	  group1_center(0),
	  group2_center(0),

	  group1(_group1),
	  group2(_group2),

	  relocation_direction(1),

	  elapsed_time(0.)
{
	#ifdef LOGGING
	Logger::counter_battles++;
	Logger::Write(LogMask::constructor, LogObject::battle, "> Creating Battle..");
	#endif

	group1->State(group1->State() | Group_State::IN_BATTLE);
	group2->State(group2->State() | Group_State::IN_BATTLE);

	#ifdef LOGGING
	Logger::Write(LogMask::constructor, LogObject::battle, "< Battle created!");
	#endif

	updater_service->Service().post(strand.wrap(boost::bind(&Battle::NeedGroup1, this)));
}

void Battle::NeedGroup1()
{
	if (group1->State() & Group_State::READY_FOR_BATTLE)
	{
		#ifdef LOGGING
		Logger::Write(LogMask::message, LogObject::battle, "< Group1 Ready!");
		#endif

		group1->Relocate(0, relocation_direction);
		group1->State(group1->State() & ~Group_State::READY_FOR_BATTLE);

		updater_service->Service().post(strand.wrap(boost::bind(&Battle::NeedGroup2, this)));
		return;
	}

	updater_service->Service().post(strand.wrap(boost::bind(&Battle::NeedGroup1, this)));
}

void Battle::NeedGroup2()
{
	if (group2->State() & Group_State::READY_FOR_BATTLE)
	{
		#ifdef LOGGING
		Logger::Write(LogMask::message, LogObject::battle, "< Group2 Ready!");
		#endif

		//TODO CHECK RELOCATION DIRECTION
		group2->Relocate(relocation_direction, relocation_direction);
		group2->State(group2->State() & ~Group_State::READY_FOR_BATTLE);

		//Join regions
		Region* base1 = group1->leader->region; for(int current = 0; current < 2 * VISION_RADIUS; ++current) base1 = base1->neighbours[1];
		Region* base2 = group2->leader->region; for(int current = 0; current < 2 * VISION_RADIUS; ++current) base2 = base2->neighbours[3];

		pair(base1, 0, base2);
		pair(base1, 7, base2->neighbours[6]);
		for (int current = 1; current < (4 * VISION_RADIUS); ++current)
		{
			base1 = base1->neighbours[6];
			base2 = base2->neighbours[6];

			pair(base1, 1, base2->neighbours[2]);
			pair(base1, 0, base2);
			pair(base1, 7, base2->neighbours[6]);
		}
		pair(base1->neighbours[6], 1, base2);
		pair(base1->neighbours[6], 0, base2->neighbours[6]);

		last_update = boost::chrono::steady_clock::now();
		updater_service->Service().post(strand.wrap(boost::bind(&Battle::Update, this)));
		return;
	}

	updater_service->Service().post(strand.wrap(boost::bind(&Battle::NeedGroup2, this)));
}

void Battle::Separate_Groups()
{
	// TODO!
}

///

void Battle::Update()
{
	boost::chrono::steady_clock::time_point now = boost::chrono::steady_clock::now();
	boost::chrono::duration< float > difference = now - last_update;
	elapsed_time += difference.count();
	last_update = now;

	if (!disposed)
	{
		if (BATTLE_UPDATE_INTERVAL < elapsed_time)
		{
			Update_Group(group1);
			Update_Group(group2);

			elapsed_time = 0.f;
		}

		updater_service->Service().post(strand.wrap(boost::bind(&Battle::Update, this)));
	}
	else
	{
		group1->State(group1->State() & ~Group_State::IN_BATTLE);
		group2->State(group2->State() & ~Group_State::IN_BATTLE);
		
		//Check if we need to create to separate Group_Unifiers
		if (!(group1->State() & Group_State::DISPOSED) && !(group2->State() & Group_State::DISPOSED))
		{
			Separate_Groups();

			Group_Unifier* unifier1 = new Group_Unifier(group1, group1->leader->region, 0);
			Group_Unifier* unifier2 = new Group_Unifier(group2, group2->leader->region, relocation_direction);
		}
		else
		{
			//Check if we create a Group_Unifier for group1 or group2
			if (group1->State() & Group_State::DISPOSED)
			{
				Group_Unifier* unifier = new Group_Unifier(group2, group2->leader->region, relocation_direction);

				updater_service->Service().post(boost::bind(&Group::Update, group1));
			}
			else
			{
				Group_Unifier* unifier = new Group_Unifier(group1, group1->leader->region, 0);

				updater_service->Service().post(boost::bind(&Group::Update, group2));
			}
		}

		delete this;
	}
}

void Battle::Update_Group(Group* _group)
{
	_group->characters.Process_Adding(this);

	_group->leader->Update(elapsed_time);
	_group->leader->region->field->Process();
	if (_group->leader->State() & Unit_State::REGION_MOVING) Update_CharacterRegion(_group->leader);

	BOOST_FOREACH(boost::shared_ptr< Character > character, _group->characters.data.list)
	{
		character->Update(elapsed_time);
		character->region->field->Process();
		if (character->State() & Unit_State::REGION_MOVING) Update_CharacterRegion(character);
	}

	_group->characters.Process_Removing(this);
}

//When a character moves from a region, this function removes the character from the subscribers of the left field, and adds it to the entered fields 
void Battle::Update_CharacterSubscription(boost::shared_ptr< Character > _character, const int& _direction)
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

bool Battle::InBounds(const int& _x, const int& _y)
{
	if (VISION_RADIUS <= _x && _x < 8 * VISION_RADIUS)
		if (VISION_RADIUS <= _y && _y <= 3 * VISION_RADIUS)
			return true;
	return false;
}

void Battle::Update_CharacterRegion(boost::shared_ptr< Character > _character)
{
	//Add and remove subscribes to regions
	int direction = StateToDirection(_character->State());

	if (!InBounds(_character->region->neighbours[direction]->x, _character->region->neighbours[direction]->y)) return;

	Update_CharacterSubscription(_character, direction);

	//Remove from current field and check if we need to process field, to send all packets
	_character->region->field->characters.Remove(_character);
	if (_character->region->field->Generated()) _character->region->field->characters.Process_Removing(); //_character->region->field

	/*//Send enter data, if we move into a loaded field
	if(_character->region->field->Generated() && _character->region->neighbours[direction]->field->Loaded())
		_character->region->neighbours[direction]->field->Broadcast(boost::shared_ptr< PacketPair >(new PacketPair(_character, new Packet(Connection::CHARACTER_ENTER, _character->GetData()))));
	
	//Send leave data, if we move out of a loaded field
	if(_character->region->field->Loaded() && _character->region->neighbours[direction]->field->Generated())
		_character->region->field->Broadcast(boost::shared_ptr< PacketPair >(new PacketPair(_character, new Packet(Connection::CHARACTER_LEAVE, _character->ID_String()))));*/

	//Update region and add to field
	_character->region = _character->region->neighbours[direction];
	_character->region->field->characters.Add(_character);
}

void Battle::ProcessAdding(boost::shared_ptr< Character > _character)
{
	//?!
}

void Battle::ProcessRemoving(boost::shared_ptr< Character > _character)
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

void Battle::Dispose()
{
	if (disposed) return; disposed = true;

	#ifdef LOGGING
	Logger::Write(LogMask::dispose, LogObject::battle, "> Disposing Battle..");
	#endif
}

Battle::~Battle()
{
	#ifdef LOGGING
	Logger::Write(LogMask::destructor, LogObject::battle, "> Battle destructor..");
	Logger::counter_battles--;
	#endif
}
