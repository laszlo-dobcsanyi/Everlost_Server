#ifndef CORE_GENERATOR_H
#define CORE_GENERATOR_H

#include <boost\thread.hpp>

class Generator
{
public:
	Generator(const int& _range);

	const int& Next();
	void Return(const int& _value);

	void Dispose();

private:
	bool disposed;

	int current;
	int* values;
	boost::shared_mutex mutex;

	Generator(const Generator& _other);
	Generator& operator=(const Generator* _other);
};

#endif