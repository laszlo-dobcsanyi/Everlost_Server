#ifndef GAME_GROUP_UNIFIER_H
#define GAME_GROUP_UNIFIER_H

#define GROUP_UNIFIER_UPDATE_INTERVAL 0.020f

#include <boost\asio.hpp>
#include <boost\thread.hpp>
#include <boost\atomic.hpp>
#include <boost\shared_ptr.hpp>

#include "Core\ListProcessable.hpp"

class Group;
class Region;
class Character;

class Group_Unifier
	: public ListProcessable_Callback< boost::shared_ptr< Character > >
{
public:
	Group_Unifier(Group* _group, Region* _center, const int& _relocation_direction);
	~Group_Unifier();

	void Update();

	void Dispose();

private:
	boost::atomic< bool > disposed;

	Group* group;

	Region* center;
	int relocation_direction;

	float elapsed_time;
	boost::asio::strand strand;
	boost::chrono::steady_clock::time_point last_update;

	void Unify();

	bool InCenter(const int& _x, const int& _y);
	bool InBounds(const int& _x, const int& _y);
	void Update_CharacterRegion(boost::shared_ptr< Character > _character);
	void Update_CharacterSubscription(boost::shared_ptr< Character > _character, const int& _direction);

	void ProcessAdding(boost::shared_ptr< Character > _character);
	void ProcessRemoving(boost::shared_ptr< Character > _character);

	Group_Unifier(const Group_Unifier& _other);
	Group_Unifier& operator=(const Group_Unifier& _other);
};
#endif