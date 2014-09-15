#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include <string>

class Object
{
public:
	Object(const float& _x, const float& _y, const int& _id);
	Object(const Object& _other);
	
#ifdef LOGGING
	~Object();
#endif

	const std::string& GetData() const { return data; };

	const float& X() const { return x; };
	void X(const float& _x) { x = _x; };

	const float& Y() const { return y; };
	void Y(const float& _y) { y = _y; };

	const int& ID() const { return id; };
	const std::string& ID_String() const { return id_string; };

private:
	float x, y;
	std::string data;

	int id;
	std::string id_string;

	Object& operator=(const Object& _other);
};
#endif