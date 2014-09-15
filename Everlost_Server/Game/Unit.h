#ifndef GAME_UNIT_H
#define GAME_UNIT_H

#include <string>
#include <stdint.h>		//uint8_t

#include <boost\atomic.hpp>

class World;
class Region;

enum Unit_State
{
	REGION_MOVING_RIGHT	= 0x01,
	REGION_MOVING_UP	= 0x02,
	REGION_MOVING_LEFT	= 0x04,
	REGION_MOVING_DOWN	= 0x08,
	REGION_MOVING		= 0x0F,
	DEAD				= 0x10,
	GROUP_CREATE		= 0x20,
	GROUP_JOIN			= 0x40
};

inline int StateToDirection(const uint8_t& _state)
{
	switch (_state)
	{
		case Unit_State::REGION_MOVING_RIGHT:									return 0;
		case Unit_State::REGION_MOVING_RIGHT| Unit_State::REGION_MOVING_UP:		return 1;
		case Unit_State::REGION_MOVING_UP:										return 2;
		case Unit_State::REGION_MOVING_UP	| Unit_State::REGION_MOVING_LEFT:	return 3;
		case Unit_State::REGION_MOVING_LEFT:									return 4;
		case Unit_State::REGION_MOVING_LEFT	| Unit_State::REGION_MOVING_DOWN:	return 5;
		case Unit_State::REGION_MOVING_DOWN:									return 6;
		case Unit_State::REGION_MOVING_DOWN	| Unit_State::REGION_MOVING_RIGHT:	return 7;

		default: return -1;
	}
}

struct Vector2
{
public:
	float x, y;

	inline Vector2(float _x, float _y) : x(_x), y(_y) { };
	inline ~Vector2() { };

private:
	Vector2(const Vector2& _other);
	Vector2& operator=(const Vector2* _other);
};

class Unit
{
public:
	World* world;
	Region* region;

	void Rotate(const float& _direction);
	void Stop();

	const std::string GetData() const;

    inline const std::string& Name() const { return name; }	
	inline const uint8_t State() const { return state; };
	inline void State(const uint8_t& _state) { state = _state; };
	inline const Vector2& Location() const { return location; };
	inline void Location(const Vector2& _location) { location = _location; };
	inline const int& ID() const { return id; };
	inline const std::string& ID_String() const { return id_string; };

	inline virtual ~Unit() { };

	virtual uint8_t Update(const float& _elapsedtime) = 0;
	void Update_Region();
	virtual void Dispose() = 0;

protected:
	boost::atomic< bool > disposed;

	int id;
	std::string id_string;
	int icon_id;
    std::string name;
	Vector2 location;
	Vector2 move_vector;
	float direction;
	float speed;
	bool moving;
	boost::atomic< uint8_t > state;

	Unit(const int _id, const std::string& _name);

	virtual void Broadcast_Location() = 0;

private:
	Unit(const Unit& _other);
	Unit& operator=(const Unit* _other);
};

#endif