#include "Core\Macro.h"
#include "Core\BattleQueue.h"

#include "Core\Processor.h"
#include "Game\Battle.h"

extern Processor* battle_queue_service;

BattleQueue::BattleQueue()
	: process_timer(new boost::asio::deadline_timer(battle_queue_service->Service(), boost::posix_time::milliseconds(PROCESS_TIME)))
{
	#ifdef LOGGING
	Logger::Write(LogMask::initialize, LogObject::battle_queue, "> BattleQueue created!");
	#endif

	process_timer->async_wait(boost::bind(&BattleQueue::Process, this, process_timer, boost::asio::placeholders::error));
}

void BattleQueue::Process(boost::asio::deadline_timer* _timer, const boost::system::error_code& _error)
{
	groups.Process_Removing();
	groups.Process_Adding();

	if (groups.data.number >= 2)
	{
		Group*  group1 = groups.data.list.front(); groups.data.list.pop_front();
		Group* group2 = groups.data.list.front(); groups.data.list.pop_front();
		groups.data.number -= 2;

		Battle* battle = new Battle(group1, group2);
	}

	_timer->expires_from_now(boost::posix_time::milliseconds(PROCESS_TIME));
	_timer->async_wait(boost::bind(&BattleQueue::Process, this, _timer, boost::asio::placeholders::error));
}

BattleQueue::~BattleQueue()
{
	delete process_timer;
}
