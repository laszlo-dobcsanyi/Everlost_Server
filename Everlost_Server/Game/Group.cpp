#include "Core\Macro.h"
#include "Game\Group.h"

#include <vector>

#include <boost\foreach.hpp>
#include <boost\lexical_cast.hpp>

#include "Core\Packet.h"
#include "Core\Processor.h"
#include "Game\World.h"
#include "Game\Region.h"
#include "Game\Field.h"
#include "Game\Character.h"
#include "Game\Connection.h"

extern Processor* updater_service;

//Creates a group for the leader and the leader's region web.
Group::Group(boost::shared_ptr< Character > _leader)
	: state(0),
	  leader(_leader),

	  strand(updater_service->Service()),
	  elapsed_time(0.),

	  invited(0)
{
	#ifdef LOGGING
	Logger::counter_groups++;
	Logger::Write(LogMask::constructor, LogObject::group, "> Creating Group for " + leader->Name() + "..");
	#endif

	//Calculate current region index
	int center_x = (int)(leader->Location().x / 512); int center_y = (int)(leader->Location().y / 512);

	//Create region web
	Region** regions = new Region*[(4 * VISION_RADIUS + 1) * (4 * VISION_RADIUS + 1)];
	for (int row = 0; row < 4 * VISION_RADIUS + 1; ++row)
		for (int column = 0; column < 4 * VISION_RADIUS + 1; ++column)
		{
			//Create new field at current location (all neighbours are set to 0 by default in constructor)
			regions[row * (4 * VISION_RADIUS + 1) + column] = new Region(center_x + column - 2 * VISION_RADIUS, center_y + row - 2 * VISION_RADIUS, leader->world);

			//Add leader to new region
			if ((VISION_RADIUS <= row && row < 3 * VISION_RADIUS + 1) && (VISION_RADIUS <= column && column < 3 * VISION_RADIUS + 1))
			{
				regions[row * (4 * VISION_RADIUS + 1) + column]->field->subscribers.Add(leader);

				if (regions[row * (4 * VISION_RADIUS + 1) + column]->field->Generated()) regions[row * (4 * VISION_RADIUS + 1) + column]->field->Process();
			}

			//There is a top-right neighbour
			if (row != 0 && column != 4 * VISION_RADIUS + 1 - 1) { pair(regions[row * (4 * VISION_RADIUS + 1) + column], 1, regions[(row - 1) * (4 * VISION_RADIUS + 1) + (column + 1)]); }

			//There is a top neighbour
			if (row != 0) { pair(regions[row * (4 * VISION_RADIUS + 1) + column], 2, regions[(row - 1) * (4 * VISION_RADIUS + 1) + column]); }

			//There is a top-left neighbour
			if (row != 0 && column != 0) { pair(regions[row * (4 * VISION_RADIUS + 1) + column], 3, regions[(row - 1) * (4 * VISION_RADIUS + 1) + column - 1]); }

			//There is a left neighbour
			if (column != 0) { pair(regions[row * (4 * VISION_RADIUS + 1) + column], 4, regions[row * (4 * VISION_RADIUS + 1) + column - 1]); }
		}

	//Set leader's region to the middle of the region web
	leader->region = regions[2 * VISION_RADIUS * (4 * VISION_RADIUS + 1) + 2 * VISION_RADIUS];

	//Add leader to it's region
	leader->region->field->characters.Add(leader);

	//Get data of fields
	//leader->region->Broadcast_EnterData(leader);

	#ifdef LOGGING
	Logger::Write(LogMask::message, LogObject::group, "< Group created!");
	#endif

	last_update = boost::chrono::steady_clock::now();

	updater_service->Service().post(strand.wrap(boost::bind(&Group::Update, this)));
}

void Group::Relocate(const int& _direction, const int& _battle_direction)
{
	//  Directions:
	//  ---------
	//	| 0 | 1 |
	//  ---------
	//	| 2 |
	//  -----
	int offset_x; int offset_y;
	switch (_direction)
	{
		case 0: offset_x = 0; offset_y = 0; break;
		case 1: offset_x = 4 * VISION_RADIUS + 1; offset_y = 0; break;
		case 2: offset_x = 0; offset_y = 4 * VISION_RADIUS + 1; break;
	}
	
	//Relocate each region of the web. Regions will relocate their fields.
	Region* base = leader->region; for (int current = 0; current < 2 * VISION_RADIUS; ++current) base = base->neighbours[3];

	//Send relocation data
	std::stringstream builder; builder << _battle_direction << ";" << (base->x - offset_x) << ";" << (base->y - offset_y);
	boost::shared_ptr< Packet > packet(new Packet(Connection::ServerCommand::BATTLE_RELOCATE, builder.str()));
	leader->connection->Send(packet);

	for (int column = 0; column < 4 * VISION_RADIUS + 1; ++column)
	{
		Region* current = base;
		for (int row = 0; row < 4 * VISION_RADIUS + 1; ++row)
		{
			current->Relocate(offset_x + row, offset_y + column);
			current = current->neighbours[0];
		}

		base = base->neighbours[6];
	}
}

void Group::Update()
{
	boost::chrono::steady_clock::time_point now = boost::chrono::steady_clock::now();
	boost::chrono::duration<float> difference = now - last_update;
	elapsed_time += difference.count();
	last_update = now;

	if (GROUP_UPDATE_INTERVAL < elapsed_time)
	{
		characters.Process_Adding(this);

		Region const * center = leader->region;

		//Update leader
		leader->Update(elapsed_time);
		if (leader->region->field->Generated()) leader->region->field->Process();

		//Update other members
		BOOST_FOREACH(boost::shared_ptr< Character > character, characters.data.list)
		{
			character->Update(elapsed_time);
			//TODO additional check if already processed field in this iteration
			if (character->region->field->Generated()) character->region->field->Process();
		}

		//Update web if leader moved
		if (leader->State() & Unit_State::REGION_MOVING)
		{
			#ifdef LOGGING
			Logger::Write(LogMask::message | LogMask::special, LogObject::group, "> Leader moving..!");
			#endif

			Update_CharacterRegion(leader);

			//Check if anyone is leaving group with the new region web
			BOOST_FOREACH(boost::shared_ptr< Character > character, characters.data.list)
			{
				if (character->State() & Unit_State::REGION_MOVING)
				{
					int direction = StateToDirection(character->State());

					//Leave group or update region if a character moves
					if (VISION_RADIUS < abs(leader->region->x - character->region->neighbours[direction]->x) || VISION_RADIUS < abs(leader->region->y - character->region->neighbours[direction]->y))
					{
						character->State(character->State() | Unit_State::GROUP_CREATE);
						characters.Remove(character);
					}
					else Update_CharacterRegion(character);
				}
				else
				{
					//Leave group if outside boundary
					if (VISION_RADIUS < abs(leader->region->x - character->region->x) || VISION_RADIUS < abs(leader->region->y - character->region->y))
					{
						character->State(character->State() | Unit_State::GROUP_CREATE);
						characters.Remove(character);
					}
				}
			}

			//Remove leaving characters first, then delete the out of range regions
			characters.Process_Removing(this);

			//Add and delete regions
			Update_Center(center, leader->region);
		}
		else
		{
			//Check other characters if they leave group
			BOOST_FOREACH(boost::shared_ptr< Character > character, characters.data.list)
			{
				if (character->State() & Unit_State::REGION_MOVING)
				{
					int direction = StateToDirection(character->State());

					//Leave group or update region if a character moves
					if (VISION_RADIUS < abs(leader->region->x - character->region->neighbours[direction]->x) || VISION_RADIUS < abs(leader->region->y - character->region->neighbours[direction]->y))
					{
						character->State(character->State() | Unit_State::GROUP_CREATE);
						characters.Remove(character);
					}
					else Update_CharacterRegion(character);
				}
			}

			characters.Process_Removing(this);
		}

		elapsed_time = 0.f;
	}

	if (state & Group_State::IN_BATTLE)	{ state |= Group_State::READY_FOR_BATTLE; return; }
	if (state & Group_State::EMPTY)		{ Dispose(); }
	if (state & Group_State::DISPOSED)	{ delete this; return; }
	
	updater_service->Service().post(strand.wrap(boost::bind(&Group::Update, this)));
}

//This function adds the subscribers to the recently created regions
void Group::Move_AddRegionSubscribers(Region* _region)
{
	//Add leader to field
	if (abs(_region->x - leader->region->x) <= VISION_RADIUS && abs(_region->y - leader->region->y) <= VISION_RADIUS) _region->field->subscribers.Add(leader);

	BOOST_FOREACH(boost::shared_ptr< Character > character, characters.data.list)
	{
		if (abs(_region->x - character->region->x) <= VISION_RADIUS && abs(_region->y - character->region->y) <= VISION_RADIUS) _region->field->subscribers.Add(character);
	}

	//If the current region's field is Generated, then we need to process it, in order to surely add to it's character data
	if (_region->field->Generated()) _region->field->characters.Process_Adding(_region->field);
}

//This function removes the subscribers from the destroyed regions
void Group::Move_RemoveRegionSubscribers(Region* _region)
{
	if(_region->field->Loaded())
	{
		//Remove leader from subscribers
		if (abs(_region->x - leader->region->x) <= VISION_RADIUS && abs(_region->y - leader->region->y) <= VISION_RADIUS) _region->field->subscribers.Remove(leader);

		BOOST_FOREACH(boost::shared_ptr< Character > character, characters.data.list)
		{
			if (abs(_region->x - character->region->x) <= VISION_RADIUS && abs(_region->y - character->region->y) <= VISION_RADIUS) _region->field->subscribers.Remove(character);
		}
	}
}

//When a character moves from a region, this function removes the character from the subscribers of the left field, and adds it to the entered fields 
void Group::Update_CharacterSubscription(boost::shared_ptr< Character > _character, const int& _direction)
{
	if (_direction % 2)
	{
		Region* added = _character->region->neighbours[neighbour_direction[_direction - 1]]; for(int distance = 0; distance < VISION_RADIUS; ++distance) added = added->neighbours[neighbour_direction[_direction - 1]];
		Region* removed = _character->region->neighbours[deleted_direction[_direction - 1]]; for(int distance = 1; distance < VISION_RADIUS; ++distance) removed = removed->neighbours[deleted_direction[_direction - 1]];

		for (int current = 0; current < 2 * VISION_RADIUS + 1; ++current)
		{
			added->field->subscribers.Add(_character); if (added->field->Generated()) added->field->subscribers.Process_Adding();
			removed->field->subscribers.Remove(_character); if (removed->field->Generated()) removed->field->subscribers.Process_Removing();

			added = added->neighbours[next[_direction - 1]];
			removed = removed->neighbours[next[_direction - 1]];
		}

		added = _character->region->neighbours[neighbour_direction[mod(_direction + 1, 8)]]; for(int distance = 1; distance < VISION_RADIUS; ++distance) added = added->neighbours[neighbour_direction[mod(_direction + 1, 8)]];
		removed = _character->region->neighbours[deleted_direction[mod(_direction + 1, 8)]]; for(int distance = 1; distance < VISION_RADIUS; ++distance) removed = removed->neighbours[deleted_direction[mod(_direction + 1, 8)]];

		added = added->neighbours[_direction];
		removed = removed->neighbours[_direction - 1];

		for (int current = 0; current < 2 * VISION_RADIUS; ++current)
		{
			added->field->subscribers.Add(_character); if (added->field->Generated()) added->field->subscribers.Process_Adding();
			removed->field->subscribers.Remove(_character); if (removed->field->Generated()) removed->field->subscribers.Process_Removing();

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
			added->field->subscribers.Add(_character); if (added->field->Generated()) added->field->subscribers.Process_Adding();
			removed->field->subscribers.Remove(_character); if (removed->field->Generated()) removed->field->subscribers.Process_Removing();

			added = added->neighbours[next[_direction]];
			removed = removed->neighbours[next[_direction]];
		}
	}
}

//When a character moves, this function removes it from it's current field and adds it to it's next field and updates the region pointer
void Group::Update_CharacterRegion(boost::shared_ptr< Character > _character)
{
	//Add and remove subscribes to regions
	int direction = StateToDirection(_character->State());

	Update_CharacterSubscription(_character, direction);

	//Remove from current field and check if we need to process field, to send all packets
	_character->region->field->characters.Remove(_character);
	if (_character->region->field->Generated()) _character->region->field->characters.Process_Removing(); //_character->region->field

	//Send enter data, if we move into a loaded field
	if(_character->region->field->Generated() && _character->region->neighbours[direction]->field->Loaded())
		_character->region->neighbours[direction]->field->Broadcast(boost::shared_ptr< PacketPair >(new PacketPair(_character, new Packet(Connection::CHARACTER_ENTER, _character->GetData()))));
	
	//Send leave data, if we move out of a loaded field
	if(_character->region->field->Loaded() && _character->region->neighbours[direction]->field->Generated())
		_character->region->field->Broadcast(boost::shared_ptr< PacketPair >(new PacketPair(_character, new Packet(Connection::CHARACTER_LEAVE, _character->ID_String()))));

	//Update region and add to field
	_character->region = _character->region->neighbours[direction];
	_character->region->field->characters.Add(_character);
}

//When the leader moves from a region, this function destroys the left, and creates the new regions
Region const * Group::Move_Center(Region const * _center, const int& _direction)
{
	if (_direction % 2)
	{
		_center = Move_Center(_center, mod(_direction - 1, 8));
		_center = Move_Center(_center, mod(_direction + 1, 8));
		return _center;
	}
	else
	{
		Region* neighbour = _center->neighbours[neighbour_direction[_direction]]; for(int distance = 1; distance < 2 * VISION_RADIUS; ++distance) neighbour = neighbour->neighbours[neighbour_direction[_direction]];
		Region* deleted = _center->neighbours[deleted_direction[_direction]]; for(int distance = 1; distance < 2 * VISION_RADIUS; ++distance) deleted = deleted->neighbours[deleted_direction[_direction]];

		Region** regions = new Region*[4 * VISION_RADIUS + 1];
		for (int current = 0; current < 4 * VISION_RADIUS + 1; ++current)
		{
			//Create region
			regions[current] = new Region(neighbour->x + new_offset[_direction][0], neighbour->y + new_offset[_direction][1], leader->world);

			//Add characters to new region
			Move_AddRegionSubscribers(regions[current]);

			//Set the 4 neighbour pointer
			if (current != 0) pair(regions[current], neighbour_pairs[0][_direction][0], neighbour->neighbours[ neighbour_pairs[0][_direction][1] ]);
			if (current != 0) pair(regions[current], neighbour_pairs[1][_direction][0], neighbour->neighbours[ neighbour_pairs[1][_direction][1] ]);
			pair(regions[current], neighbour_pairs[2][_direction][0], neighbour);
			if (current != 4 * VISION_RADIUS) pair(regions[current], neighbour_pairs[3][_direction][0], neighbour->neighbours[ neighbour_pairs[3][_direction][1] ]);

			neighbour = neighbour->neighbours[next[_direction]];

			//Set the 3 neighbour to null pointer
			if (current != 0) deleted->neighbours[ deleted_pairs[0][_direction][0] ]->neighbours[ deleted_pairs[0][_direction][1] ] = 0;

			deleted->neighbours[ deleted_pairs[1][_direction][0] ]->neighbours[ deleted_pairs[1][_direction][1] ] = 0;

			if (current != 4 * VISION_RADIUS) deleted->neighbours[ deleted_pairs[2][_direction][0] ]->neighbours[ deleted_pairs[2][_direction][1] ] = 0; 

			//Delete region
			Move_RemoveRegionSubscribers(deleted);

			Region* tmp = deleted;
			deleted = deleted->neighbours[ next[_direction] ];
			delete tmp;
		}

		return _center->neighbours[_direction];
	}
}

//When the leader moves from a region, this function destroys the left, and creates the new regions
void Group::Update_Center(Region const * _old, const Region* const _new)
{
	while (_old->x < _new->x) _old = Move_Center(_old, 0);
	while (_new->x < _old->x) _old = Move_Center(_old, 4);
	while (_new->y < _old->y) _old = Move_Center(_old, 2);
	while (_old->y < _new->y) _old = Move_Center(_old, 6);
}

///

void Group::ProcessAdding(boost::shared_ptr< Character > _character)
{
	//Set region to leader's region
	_character->region = leader->region;

	//Add to subscribed regions
	Region* base = leader->region; for (int current = 0; current < VISION_RADIUS; ++current) base = base->neighbours[3];
	for (int row = 0; row < 2 * VISION_RADIUS + 1; ++row)
	{
		Region* current = base;
		for (int column = 0; column < 2 * VISION_RADIUS + 1; ++column)
		{
			current->field->subscribers.Add(_character);
			if (current->field->Generated()) current->field->Process();

			current = current->neighbours[0];
		}
		base = base->neighbours[6];
	}

	//Add character to leader's region
	_character->region->field->characters.Add(_character);

	//Teleport to leader
	_character->Teleport(leader->Location());

	//Broadcast adding
	leader->connection->Send(Connection::GROUP_ADD, _character->GetData());
	_character->connection->Send(Connection::GROUP_ADD, leader->GetData());
	_character->connection->Send(Connection::GROUP_LEADER, leader->ID_String());

	BOOST_FOREACH(boost::shared_ptr< Character > character, characters.data.list)
	{
		if (character != _character)
		{
			//Send added character's data
			character->connection->Send(Connection::GROUP_ADD, _character->GetData());
			//Send other member's data
			_character->connection->Send(Connection::GROUP_ADD, character->GetData());
		}
	}
}

void Group::ProcessRemoving(boost::shared_ptr< Character > _character)
{
	//Broadcast leaving


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
	if (_character == leader)
	{
		if (0 < characters.data.number)
		{
			leader = characters.data.list.front();
			characters.data.list.pop_front();
			--characters.data.number;

			//Send leaver and leader data
			_character->connection->Send(Connection::GROUP_LEAVE, _character->ID_String());

			leader->connection->Send(Connection::GROUP_LEAVE, _character->ID_String());
			leader->connection->Send(Connection::GROUP_LEADER, leader->ID_String());

			BOOST_FOREACH(boost::shared_ptr< Character > character, characters.data.list)
			{
				character->connection->Send(Connection::GROUP_LEAVE, _character->ID_String());
				character->connection->Send(Connection::GROUP_LEADER, leader->ID_String());
			}
		}
		else
		{
			Dispose();
		}
	}
	else
	{
		//Send leave data
		leader->connection->Send(Connection::GROUP_LEAVE, _character->ID_String());

		BOOST_FOREACH(boost::shared_ptr< Character > character, characters.data.list)
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

///

void Group::Invite(boost::shared_ptr< Character > _character)
{
	if (!invited && _character->group != this && characters.data.number + characters.adding.number - characters.removing.number < GROUP_CHARACTERS_NUMBER - 1)
	{
		invited = _character;
		invited->inviter = this;

		invited->connection->Send(Connection::GROUP_INVITATION, leader->Name());
	}
}

void Group::Invitation_Join()
{
	invited->State(invited->State() | Unit_State::GROUP_JOIN);
	invited->group->characters.Remove(invited);
}

void Group::Invitation_Decline()
{
	invited->inviter = 0;
	invited = 0;
}

///

void Group::Dispose()
{
	if (state & Group_State::DISPOSED) return; state |= Group_State::DISPOSED;

	#ifdef LOGGING
	Logger::Write(LogMask::dispose, LogObject::group, "> Disposing Group..");
	#endif

	if (invited) invited->inviter = 0;

	if (!(state & Group_State::EMPTY))
	{
		Region* first = leader->region; for(int current = 0; current < VISION_RADIUS * 2; ++current) first = first->neighbours[3];
		for(int column = 0; column < VISION_RADIUS * 4 + 1; ++column)
		{
			Region* current = first;
			first = first->neighbours[6];

			for(int row = 0; row < VISION_RADIUS * 4 + 1; ++row)
			{
				Region* tmp = current;
				current = current->neighbours[0];
				delete tmp;
			}
		}
	}
}

Group::~Group()
{
	#ifdef LOGGING
	Logger::Write(LogMask::destructor, LogObject::group, "> Group destructor..");
	Logger::counter_groups--;
	#endif
}
