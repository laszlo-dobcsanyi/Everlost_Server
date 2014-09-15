#include "Core\Macro.h"

#define NETWORK_THREADS 2
#define UPDATER_THREADS 4
#define BATTLE_QUEUE_THREADS 1

#include "Core\Realm.h"
#include "Core\Gateway.h"
#include "Core\Registry.h"
#include "Core\BattleQueue.h"
#include "Core\Processor.h"

void Banner();
bool HandleCommand(const std::string& _command);

Realm* realm;
Gateway* gateway;
Registry* registry;
BattleQueue* battle_queue;

Processor* network_service;
Processor* updater_service;
Processor* battle_queue_service;

int main()
{
	Banner();

	#ifdef LOGGING
	Logger::Write(LogMask::initialize, LogObject::core, "> Configurating..");
	#endif

	updater_service = new Processor(UPDATER_THREADS);
	network_service = new Processor(NETWORK_THREADS);
	battle_queue_service = new Processor(BATTLE_QUEUE_THREADS);

	realm = new Realm();
	registry = new Registry("logindata.db");
	battle_queue = new BattleQueue();
	gateway = new Gateway(boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(GATEWAY_ADDRESS), 1425));

	#ifdef LOGGING
	Logger::Write(LogMask::initialize, LogObject::core, "< Configurated!");
	#endif

	std::string command;
	do
	{
		std::cin >> command;
	} while(HandleCommand(command));
}

void Banner()
{
	#ifdef LOGGING
	Logger::SetColor(0x0F);
	#endif
	std::cout << "                      _____               _           _   " << std::endl;
	std::cout << "                     |  ___|             | |         | |  " << std::endl;
	std::cout << "                     | |____   _____ _ __| | ___  ___| |_ " << std::endl;
	std::cout << "                     |  __\\ \\ / / _ \\ '__| |/ _ \\/ __| __|" << std::endl;
	std::cout << "                     | |___\\ V /  __/ |  | | (_) \\__ \\ |_ " << std::endl;
	std::cout << "                     \\____/ \\_/ \\___|_|  |_|\\___/|___/\\__|" << std::endl << std::endl << std::endl;
}

bool HandleCommand(const std::string& _command)
{
	#ifdef LOGGING
	if (_command == "counters")
	{
		std::stringstream counters; counters << "> COUNTERS <\n";
		counters << "> Packets: " << Logger::counter_packets << std::endl;
		counters << "> Packet Pairs: " << Logger::counter_packet_pairs << std::endl << std::endl;
		counters << "> Connections " << Logger::counter_connections << std::endl;
		counters << "> Characters " << Logger::counter_characters << std::endl << std::endl;
		counters << "> Groups: " << Logger::counter_groups << std::endl;
		counters << "> Groups Unifiers: " << Logger::counter_group_unifiers << std::endl;
		counters << "> Battles: " << Logger::counter_battles << std::endl << std::endl;
		counters << "> Regions " << Logger::counter_regions << std::endl;
		counters << "> Field_Generateds " << Logger::counter_field_generateds << std::endl;
		counters << "> Objects " << Logger::counter_objects << std::endl << std::endl;
		counters << "> Lockable Lists " << Logger::counter_list_lockable << std::endl;
		counters << "> Processable Lists " << Logger::counter_list_processable << std::endl << std::endl;

		std::cout << counters.str();
		return true;
	}

	if (_command == "all")
	{
		Logger::mask = 0xFFFFFFFF;
		Logger::object = 0xFFFFFFFF;
		return true;
	}

	if (_command == "none")
	{
		Logger::mask = 0;
		Logger::object = 0;
		return true;
	}

	if (_command == "memory")
	{
		Logger::mask = LogMask::constructor | LogMask::destructor;
		Logger::object = ~LogObject::field_generated;// 0xFFFFFFFF;
		return true;
	}

	if (_command == "special")
	{
		Logger::mask = LogMask::special;
		Logger::object = 0xFFFFFFFF;
	}

	if (_command == "error")
	{
		Logger::mask = LogMask::error | LogMask::fatal_error;
		Logger::object = 0xFFFFFFFF;
	}

	if (_command == "query")
	{
		std::string user; std::cin >> user;
		std::cout << registry->GetNode(user);
	}


	#endif

	if (_command == "exit")
	{
		return false;
	}

	return true;
}

