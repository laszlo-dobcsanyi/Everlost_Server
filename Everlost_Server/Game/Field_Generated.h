#ifndef FIELD_GENERATED_H
#define FIELD_GENERATED_H

#include <boost\atomic.hpp>

#include "Game\Field.h"

class Field_Generated
	: public Field
{
public:
	Field_Generated();
	~Field_Generated();
	inline Field_Generated* Generated() { return this; };

	void Process();

	void ProcessAdding(boost::shared_ptr< Character > _character);
	void ProcessRemoving(boost::shared_ptr< Character > _character);

	void Dispose();

private:
	boost::atomic< bool > disposed;

	Field_Generated(const Field_Generated& _other);
	Field_Generated& operator=(const Field_Generated* _other);
};

#endif