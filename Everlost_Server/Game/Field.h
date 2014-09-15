#ifndef GAME_FIELD_H
#define GAME_FIELD_H

#include <boost\shared_ptr.hpp>

#include "Core\ListProcessable.hpp"

struct PacketPair;

class Object;
class Character;

class Field_Loaded;
class Field_Boundary;
class Field_Generated;

class Field
	: public ListProcessable_Callback< boost::shared_ptr< Character > >
{
public:
	std::vector<Object*> objects;
	
	ListProcessable< boost::shared_ptr< Character> > characters;
	ListProcessable< boost::shared_ptr< Character> > subscribers;

	inline virtual ~Field() { };

	virtual void Process() = 0;

	void Relocate(const int& _offset_x, const int& _offset_y);

	virtual void ProcessAdding(boost::shared_ptr< Character > _character) = 0;
	virtual void ProcessRemoving(boost::shared_ptr< Character > _character) = 0;

	inline void Send(boost::shared_ptr< PacketPair > _packet_pair) { if (0 < characters.data.number) sends.Add(_packet_pair); }
	inline void Broadcast(boost::shared_ptr< PacketPair > _packet_pair) { if (0 < subscribers.data.number) broadcasts.Add(_packet_pair); }

	virtual void Broadcast_Enter(boost::shared_ptr< Character > _character);
	virtual void Broadcast_Leave(boost::shared_ptr< Character > _character);

	inline virtual Field_Loaded* Loaded() { return 0; };
	inline virtual Field_Boundary* Boundary() { return 0; };
	inline virtual Field_Generated* Generated() { return 0; };

protected:
	ListLockable< boost::shared_ptr< PacketPair > > sends;
	ListLockable< boost::shared_ptr< PacketPair > > broadcasts;

	Field();

	void Process_Messages();
	
private:
	Field(const Field& _other);
	Field& operator=(const Field* _other);
};
#endif