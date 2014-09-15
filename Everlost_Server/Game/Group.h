#ifndef GAME_GROUP_H
#define GAME_GROUP_H

#define GROUP_UPDATE_INTERVAL 0.020f
#define GROUP_CHARACTERS_NUMBER 5

#include <stdint.h>

#include <boost\asio.hpp>
#include <boost\thread.hpp>
#include <boost\atomic.hpp>
#include <boost\shared_ptr.hpp>

#include "Core\ListProcessable.hpp"

class World;
class Region;
class Character;

enum Group_State
{
	NONE				= 0x00,
	DISPOSED			= 0x01 << 0,
	IN_BATTLE			= 0x01 << 1,
	READY_FOR_BATTLE	= 0x01 << 2,
	EMPTY				= 0x01 << 3
};

static const int neighbour_direction[8] = { mod(0 + 1, 8), mod(1 + 1, 8), mod(2 + 1, 8), mod(3 + 1, 8), mod(4 + 1, 8), mod(5 + 1, 8), mod(6 + 1, 8), mod(7 + 1, 8) };
static const int deleted_direction[8] = { mod(0 + 3, 8), mod(1 + 3, 8), mod(2 + 3, 8), mod(3 + 3, 8), mod(4 + 3, 8), mod(5 + 3, 8), mod(6 + 3, 8), mod(7 + 3, 8) };

static const int new_offset[8][2] = { {+1, 0}, {0, 0}, {0, -1}, {0, 0}, {-1, 0}, {0, 0}, {0, 1}, {0, 0} };

static const int next[8] = { mod(0 + 6, 8), 0, mod(2 + 6, 8), 0, mod(4 + 6, 8), 0, mod(6 + 6, 8), 0 };

static const int neighbour_pairs[4][8][2] = { { { mod(0 + 2, 8), mod(0 + 1, 8) }, { 0, 0 }, { mod(2 + 2, 8), mod(2 + 1, 8) }, { 0, 0 }, { mod(4 + 2, 8), mod(4 + 1, 8) }, { 0, 0 }, { mod(6 + 2, 8), mod(6 + 1, 8) }, { 0, 0 } },
	                                            { { mod(0 + 3, 8), mod(0 + 2, 8) }, { 0, 0 }, { mod(2 + 3, 8), mod(2 + 2, 8) }, { 0, 0 }, { mod(4 + 3, 8), mod(4 + 2, 8) }, { 0, 0 }, { mod(6 + 3, 8), mod(6 + 2, 8) }, { 0, 0 } },
												{ { mod(0 + 4, 8), 0 }, { 0, 0 }, { mod(2 + 4, 8), 0 }, { 0, 0 }, { mod(4 + 4, 8), 0 }, { 0, 0 }, { mod(6 + 4, 8), 0 }, { 0, 0 } },
												{ { mod(0 + 5, 8), mod(0 + 6, 8) }, { 0, 0 }, { mod(2 + 5, 8), mod(2 + 6, 8) }, { 0, 0 }, { mod(4 + 5, 8), mod(4 + 6, 8) }, { 0, 0 }, { mod(6 + 5, 8), mod(6 + 6, 8) }, { 0, 0 } } };

static const int deleted_pairs[3][8][2] =   { { { mod(0 + 1, 8), mod(0 + 5, 8) }, { 0, 0 }, { mod(2 + 1, 8), mod(2 + 5, 8) }, { 0, 0 }, { mod(4 + 1, 8), mod(4 + 5, 8) }, { 0, 0 }, { mod(6 + 1, 8), mod(6 + 5, 8) }, { 0, 0 } },
												{ { mod(0 + 0, 8), mod(0 + 4, 8) }, { 0, 0 }, { mod(2 + 0, 8), mod(2 + 4, 8) }, { 0, 0 }, { mod(4 + 0, 8), mod(4 + 4, 8) }, { 0, 0 }, { mod(6 + 0, 8), mod(6 + 4, 8) }, { 0, 0 } },
												{ { mod(0 + 7, 8), mod(0 + 3, 8) }, { 0, 0 }, { mod(2 + 7, 8), mod(2 + 3, 8) }, { 0, 0 }, { mod(4 + 7, 8), mod(4 + 3, 8) }, { 0, 0 }, { mod(6 + 7, 8), mod(6 + 3, 8) }, { 0, 0 } } };

class Group
	: public ListProcessable_Callback< boost::shared_ptr< Character > >
{
public:
	boost::shared_ptr< Character > leader;
	ListProcessable< boost::shared_ptr< Character > > characters;

	Group(boost::shared_ptr< Character > _leader);
	~Group();

	inline const uint8_t State() const { return state; };
	inline void State(const uint8_t& _state) { state = _state; };

	void Update();

	void Relocate(const int& _direction, const int& _battle_direction);

	boost::shared_ptr< Character > invited;

	void Invite(boost::shared_ptr< Character > _character);
	void Invitation_Join();
	void Invitation_Decline();
	
	void Dispose();

private:
	boost::atomic< uint8_t > state;

	boost::asio::strand strand;
	boost::chrono::steady_clock::time_point last_update;
	float elapsed_time;

	void Update_Center(Region const * _old, const Region* const _new);
	void Update_CharacterRegion(boost::shared_ptr< Character > _character);
	void Update_CharacterSubscription(boost::shared_ptr< Character > _character, const int& _direction);

	void Move_AddRegionSubscribers(Region* _region);
	void Move_RemoveRegionSubscribers(Region* _region);
	Region const * Move_Center(Region const * _center, const int& _direction);

	void ProcessAdding(boost::shared_ptr< Character > _character);
	void ProcessRemoving(boost::shared_ptr< Character > _character);

	Group(const Group& _other);
	Group& operator=(const Group* _other);
};

#endif