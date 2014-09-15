#include "Core\Macro.h"
#include "Core\Processor.h"

#include <boost\bind.hpp>

Processor::Processor(const int& _threads)
	: service(),
	  work(service)
{
	for(int current = 0; current < _threads; ++current)
		threads.create_thread(boost::bind(&boost::asio::io_service::run, &service));
}