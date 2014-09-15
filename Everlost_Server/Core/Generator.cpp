#include "Core\Macro.h"
#include "Core\Generator.h"

/// PUBLIC

Generator::Generator(const int& _range)
	: disposed(false),
	  current(_range)
{
	values = new int[_range];
	for(int current = 0; current < _range; ++current)
		values[current] = current;
}

const int& Generator::Next()
{
	boost::unique_lock<boost::shared_mutex> lock(mutex);
	current--;
	if (current < 0)
	{
		#ifdef LOGGING
		Logger::Write(LogMask::fatal_error, LogObject::generator, "# Error @ Generator::Next -> Generator out of range!");
		#endif
		return values[0];
	}

	return values[current];
}

void Generator::Return(const int& _value)
{
	boost::unique_lock<boost::shared_mutex> lock(mutex);
	values[current] = _value;
	current++;
}

void Generator::Dispose()
{
	if (disposed) return; disposed = true;
	delete[] values;
}
