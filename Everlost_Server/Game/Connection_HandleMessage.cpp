#include "Core\Macro.h"
#include "Game\Connection.h"

#include <vector>

#include <boost\lexical_cast.hpp>
#include <boost\algorithm\string.hpp>

#include "Core\Registry.h"
#include "Core\BattleQueue.h"
#include "Game\Group.h"
#include "Game\Character.h"

extern Registry* registry;
extern BattleQueue* battle_queue;

std::string Connection::ToString(char* _data, unsigned int _size)
{
	std::string value = "";
	for(unsigned int current = 0; current < _size; ++current) value += _data[current];
	return value;
}

void Connection::HandleMessage(size_t _received)
{
	int command = 0x00000000; command |= data[0] << 24; command |= data[1] << 16; command |= data[2] << 8; command |= data[3];
	std::vector<std::string> arguments; boost::split(arguments, ToString(data + 4, _received - 4), boost::is_any_of(";"));

	if (!character) switch (command)
	{
		case ClientCommand::LOGIN_RESPONSE:
			{
				boost::shared_ptr< Character > hero = boost::shared_ptr< Character >(new Character(registry_node->user, this));
				hero->group = new Group(hero);
				character = hero;
			}
			break;

	}
	else switch(command)
	{
		case ClientCommand::HERO_MOVE:
			#ifdef LOGGING
			Logger::Write(LogMask::message, LogObject::connection, "> Moving hero..");
			#endif

			character->Rotate(boost::lexical_cast<float>(arguments[0].data())); 
			break;

		case ClientCommand::HERO_STOP:
			#ifdef LOGGING
			Logger::Write(LogMask::message, LogObject::connection, "> Stopping hero..");
			#endif
			
			character->Stop();
			break;

		///

		case ClientCommand::GROUP_INVITE:
			{
				ABC_Table_Node* node = registry->GetNode(arguments[0]);
				if (node)
				{
					if (node->connection)
					{
						if (node->connection != this) character->group->Invite(node->connection->character);
							//node->connection->Send(Connection::ServerCommand::GROUP_INVITATION, character->Name());
					}
				}
			}
			break;

		case ClientCommand::GROUP_ACCEPT:
			if (character->inviter)
			{
				character->inviter->Invitation_Join();
			}
			break;

		case ClientCommand::GROUP_DECLINE:
			if (character->inviter)
			{
				character->inviter->Invitation_Decline();
			}
			break;

		///

		case ClientCommand::BATTLE_JOIN:
			battle_queue->groups.Add(character->group);
			break;

		///

		case ClientCommand::PING: break;

		default:
			#ifdef LOGGING
			Logger::Write(LogMask::error, LogObject::connection, "# Error @ Connection::HandleMessage: Unkown command from connection(" + boost::lexical_cast<std::string>(command) + ")!");
			#endif
			break;
	}
}
