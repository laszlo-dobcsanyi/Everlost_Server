#include "Core\Macro.h"
#include "Core\Registry.h"

#include <fstream>

#include <boost\algorithm\string.hpp>

/// Top level

ABC_Table_Level3::ABC_Table_Level3()
	 : head(0)
{

}

ABC_Table_Node* ABC_Table_Level3::GetNode(const std::string& _user)
{
	boost::unique_lock<boost::shared_mutex> lock(mutex);
		
	ABC_Table_Node* current = head;
	while(current) { if (current->user == _user) return current; current = current->next; }

	return 0;
}

ABC_Table_Level3::~ABC_Table_Level3()
{
	boost::unique_lock<boost::shared_mutex> lock(mutex);

	ABC_Table_Node* current = head;
	while(current)
	{
		ABC_Table_Node* tmp = current;
		current = current->next;
		delete tmp;
	}
};

/// Registry

Registry::Registry(const std::string& _file)
{
	#ifdef LOGGING
	Logger::Write(LogMask::constructor, LogObject::registry, "\t> Allocating registry table..");
	#endif

	table = new ABC_Table();

	#ifdef LOGGING
	Logger::Write(LogMask::constructor, LogObject::registry, "\t> Loading registry data from " + _file +"..");
	#endif

	std::ifstream data;
	data.open(_file);

	if (data)
	{
		std::string tmp;
		while(data >> tmp)
		{
			std::vector< std::string > args;
			boost::split(args, tmp, boost::is_any_of(";"));
		
			ABC_Table_Node* node = new ABC_Table_Node(args[0], args[1]);
			node->next = table->level[args[0][0]-'a']->level[args[0][1]-'a']->level[args[0][2]-'a']->head;
			table->level[args[0][0]-'a']->level[args[0][1]-'a']->level[args[0][2]-'a']->head = node;
		}
	}
	else
	{
		#ifdef LOGGING
		Logger::Write(LogMask::fatal_error, LogObject::registry, "# Fatal error while opening registry data!");
		#endif
	}

	data.close();

	#ifdef LOGGING
	Logger::Write(LogMask::constructor, LogObject::registry, "> Registry loaded!");
	#endif
}

bool Registry::AddNode(const std::string& _user, const std::string& _pass)
{
	ABC_Table_Level3* container = table->level[_user[0]-'a']->level[_user[1]-'a']->level[_user[2]-'a'];
	boost::unique_lock<boost::shared_mutex> lock(container->mutex);

	ABC_Table_Node* current = container->head;
	while(current)
	{ 
		if (current->user == _user)
		{
			#ifdef LOGGING
			Logger::Write(LogMask::error, LogObject::registry, "# User with the same name already exists!");
			#endif
			return false;
		}
		current = current->next;
	}

	ABC_Table_Node* node = new ABC_Table_Node(_user, _pass);
	node->next = container->head;
	container->head = node;
	return true;
}

