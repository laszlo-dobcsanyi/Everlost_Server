#ifndef CORE_BATTLE_QUEUE_H
#define CORE_BATTLE_QUEUE_H

#define PROCESS_TIME 3 * 1000

#include "Core\ListProcessable.hpp"

#include <boost\asio.hpp>
#include <boost\shared_ptr.hpp>

class Group;

class BattleQueue
{
public:
	ListProcessable< Group* > groups;

	BattleQueue();
	~BattleQueue();

private:
	boost::asio::deadline_timer* process_timer;

	void Process(boost::asio::deadline_timer* _timer, const boost::system::error_code& _error);

	BattleQueue(const BattleQueue& _other);
	BattleQueue& operator=(const BattleQueue& _other);
};

#endif