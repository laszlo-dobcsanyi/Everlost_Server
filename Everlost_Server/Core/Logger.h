#ifndef CORE_LOGGER_H
#define CORE_LOGGER_H

#include <iostream>
#include <string>

#include <boost\atomic.hpp>

namespace LogMask
{
	enum LogMask_Enum
	{
		constructor		= 1 << 1,
		destructor		= 1 << 2,
		dispose			= 1 << 3,
		error			= 1 << 4,
		fatal_error		= 1 << 5,
		message			= 1 << 6,
		initialize		= 1 << 7,
		special			= 1 << 8
	};
};

namespace LogObject
{
	enum LogObject_Enum
	{
		core			= 1 << 1,
		gateway			= 1 << 2,
		generator		= 1 << 3,
		realm			= 1 << 4,
		character		= 1 << 5,
		connection		= 1 << 6,
		field_generated	= 1 << 7,
		field_loaded	= 1 << 8,
		group			= 1 << 9,
		group_unifier	= 1 << 10,
		item			= 1 << 11,
		region			= 1 << 12,
		world			= 1 << 13,
		processor		= 1 << 14,
		registry		= 1 << 15,
		battle_queue	= 1 << 16,
		battle			= 1 << 17,
		object			= 1 << 18
	};
};

class Logger
{
public:
    static int mask;
	static int object;

	typedef unsigned int unsigned_int;

	static boost::atomic< unsigned_int > counter_packets;
	static boost::atomic< unsigned_int > counter_packet_pairs;

	static boost::atomic< unsigned_int > counter_characters;
	static boost::atomic< unsigned_int > counter_connections;

	static boost::atomic< unsigned_int > counter_groups;
	static boost::atomic< unsigned_int > counter_group_unifiers;
	static boost::atomic< unsigned_int > counter_battles;

	static boost::atomic< unsigned_int > counter_regions;
	static boost::atomic< unsigned_int > counter_field_generateds;
	static boost::atomic< unsigned_int > counter_objects;

	static boost::atomic< unsigned_int > counter_list_lockable;
	static boost::atomic< unsigned_int > counter_list_processable;

    static void Write(const int& _mask, const int& _object, const std::string& _msg);
	static void SetColor(const unsigned short& _color);

private:
	static void* console;

	Logger();
};

#endif