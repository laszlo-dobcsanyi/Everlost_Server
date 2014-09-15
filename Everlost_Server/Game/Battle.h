#ifndef GAME_BATTLE_H
#define GAME_BATTLE_H

#define BATTLE_UPDATE_INTERVAL 0.020f

#include <boost\asio.hpp>
#include <boost\thread.hpp>
#include <boost\atomic.hpp>

#include "Core\ListProcessable.hpp"

class Region;
class Group;
class Character;

class Battle
	: public ListProcessable_Callback< boost::shared_ptr< Character > >
{
public:
	Battle(Group* _group1, Group* _group2);
	~Battle();

private:
	boost::atomic< bool > disposed;

	Group* group1;
	Group* group2;

	Region* group1_center;
	Region* group2_center;

	int relocation_direction;

	float elapsed_time;
	boost::asio::strand strand;
	boost::chrono::steady_clock::time_point last_update;

	void NeedGroup1();
	void NeedGroup2();

	void Separate_Groups();

	bool InBounds(const int& _x, const int& _y);

	void Update();
	void Update_Group(Group* _group);

	void Update_CharacterRegion(boost::shared_ptr< Character > _character);
	void Update_CharacterSubscription(boost::shared_ptr< Character > _character, const int& _direction);

	void ProcessAdding(boost::shared_ptr< Character > _character);
	void ProcessRemoving(boost::shared_ptr< Character > _character);

	void Dispose();

	Battle(const Battle& _other);
	Battle& operator=(const Battle& _other);
};
#endif