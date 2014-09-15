#ifndef GAME_FIELD_LOADED_H
#define GAME_FIELD_LOADED_H

#define FIELD_UPDATE_INTERVAL 20

#include <boost\thread.hpp>

#include "Game\Field.h"

class Field_Loaded
	: public Field
{
public:
	Field_Loaded();
	inline ~Field_Loaded() { };
	inline Field_Loaded* Loaded() { return this; };

	void Process();

	void ProcessAdding(boost::shared_ptr< Character > _character);
	void ProcessRemoving(boost::shared_ptr< Character > _character);

private:
	boost::thread thread;

	void Update();

	Field_Loaded(const Field_Loaded& _other);
	Field_Loaded& operator=(const Field_Loaded* _other);
};

#endif