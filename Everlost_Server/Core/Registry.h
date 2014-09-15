#ifndef CORE_REGISTRY_H
#define CORE_REGISTRY_H

#include <boost\thread.hpp>
#include <boost\shared_ptr.hpp>

class Connection;

struct ABC_Table_Node
{
public:
	ABC_Table_Node* next;

	std::string user;
	std::string pass;
	Connection* connection;

	ABC_Table_Node() : next(0), connection(0) { };
	ABC_Table_Node(const std::string& _user, const std::string& _pass) : user(_user), pass(_pass), next(0), connection(0) { };
	~ABC_Table_Node() { /*character.reset();*/ };

private:
	ABC_Table_Node(const ABC_Table_Node& _other);
	ABC_Table_Node& operator=(const ABC_Table_Node& _other);
};

struct ABC_Table_Level3
{
public:
	ABC_Table_Node* head;
	boost::shared_mutex mutex;

	ABC_Table_Level3();
	~ABC_Table_Level3();

	ABC_Table_Node* GetNode(const std::string& _user);

private:
	ABC_Table_Level3(const ABC_Table_Level3& _other);
	ABC_Table_Level3& operator=(const ABC_Table_Level3& _other);
};

struct ABC_Table_Level2
{
public:
	ABC_Table_Level3* level[26];

	ABC_Table_Level2() { for(int current = 0; current < 26; ++current) level[current] = new ABC_Table_Level3(); };
	~ABC_Table_Level2() { for(int current = 0; current < 26; ++current) delete level[current]; };

private:
	ABC_Table_Level2(const ABC_Table_Level2& _other);
	ABC_Table_Level2& operator=(const ABC_Table_Level2& _other);
};

struct ABC_Table_Level1
{
public:
	ABC_Table_Level2* level[26];

	ABC_Table_Level1() { for(int current = 0; current < 26; ++current) level[current] = new ABC_Table_Level2(); };
	~ABC_Table_Level1() { for(int current = 0; current < 26; ++current) delete level[current]; };

private:
	ABC_Table_Level1(const ABC_Table_Level1& _other);
	ABC_Table_Level1& operator=(const ABC_Table_Level1& _other);
};

struct ABC_Table
{
public:
	ABC_Table_Level1* level[26];

	ABC_Table() { for(int current = 0; current < 26; ++current) level[current] = new ABC_Table_Level1(); };
	~ABC_Table() { for(int current = 0; current < 26; ++current) delete level[current]; };

private:
	ABC_Table(const ABC_Table& _other);
	ABC_Table& operator=(const ABC_Table& _other);
};

///

class Registry
{
public:
	Registry(const std::string& _file);
	~Registry() { delete table; };

	bool AddNode(const std::string& _user, const std::string& _pass);
	ABC_Table_Node* GetNode(const std::string& _user) { return table->level[_user[0]-'a']->level[_user[1]-'a']->level[_user[2]-'a']->GetNode(_user.data()); };

private:
	ABC_Table* table;

	Registry(const Registry& _other);
	Registry& operator=(const Registry& _other);
};

#endif