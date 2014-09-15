#include "Core\Macro.h"
#include "Core\Logger.h"

#include "Windows.h"

int Logger::mask	=	0xFFFFFFFF;//LogMask::special; 
int Logger::object	=  ~LogObject::field_generated; //0xFFFFFFFF;
HANDLE Logger::console = GetStdHandle(STD_OUTPUT_HANDLE);

static const WORD colors[] = {  0x0F,
								0x0C, 0x04, 0x04,
								0x0A, 0x02, 0x02,
								0x0B, 0x03, 0x03};

typedef unsigned int unsigned_int;

boost::atomic< unsigned_int > Logger::counter_packets(0);
boost::atomic< unsigned_int > Logger::counter_packet_pairs(0);

boost::atomic< unsigned_int > Logger::counter_characters(0);
boost::atomic< unsigned_int > Logger::counter_connections(0);
	
boost::atomic< unsigned_int > Logger::counter_groups(0);
boost::atomic< unsigned_int > Logger::counter_group_unifiers(0);
boost::atomic< unsigned_int > Logger::counter_battles(0);

boost::atomic< unsigned_int > Logger::counter_regions(0);
boost::atomic< unsigned_int > Logger::counter_field_generateds(0);
boost::atomic< unsigned_int > Logger::counter_objects(0);

boost::atomic< unsigned_int > Logger::counter_list_lockable(0);
boost::atomic< unsigned_int > Logger::counter_list_processable(0);

void Logger::Write(const int& _mask, const int& _object, const std::string& _msg)
{
	//bit 0 - foreground blue
	//bit 1 - foreground green
	//bit 2 - foreground red
	//bit 3 - foreground intensity

	//bit 4 - background blue
	//bit 5 - background green
	//bit 6 - background red
	//bit 7 - background intensity


	if((mask & _mask) && (object & _object))
	{
		//SetConsoleTextAttribute(console, ((0 <= _level) &&(_level <= 9)) ? colors[_level] : 0x0F);
		std::cout << _msg.c_str() << std::endl;
	}
}

void Logger::SetColor(const WORD& _color)
{
	SetConsoleTextAttribute(console, _color);
}


