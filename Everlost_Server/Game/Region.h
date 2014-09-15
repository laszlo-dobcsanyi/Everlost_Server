#ifndef GAME_REGION_H
#define GAME_REGION_H

#define VISION_RADIUS 2

#include <boost\shared_ptr.hpp>

class World;
class Field;
struct Packet;
class Character;

class Region
{
public:
	int x, y;

	Field* field;

	// 3 2 1
	// 4 X 0
	// 5 6 7
	Region* neighbours[8];

	Region(const int& _x, const int& _y, Field* _field);
	Region(const int& _x, const int& _y, World* _world);

	void Relocate(const int& _x, const int& _y);

	~Region();

private:
	Region(const Region& _other);
	Region& operator=(const Region* _other);
};

inline void pair(Region* _region1, int _direction, Region* _region2) { _region1->neighbours[_direction] = _region2; if (_region2) _region2->neighbours[reverse(_direction)] = _region1; };

#endif