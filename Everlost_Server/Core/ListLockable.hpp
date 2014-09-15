#ifndef CORE_LIST_LOCKABLE_HPP
#define CORE_LIST_LOCKABLE_HPP

#include <list>

#include <boost\thread.hpp>
#include <boost\atomic.hpp>

template<class T> class ListLockable
{
public:
	boost::atomic< int > number;
	std::list<T> list;
	boost::shared_mutex mutex;

	ListLockable() : number(0)
	{
		#ifdef LOGGING
		Logger::counter_list_lockable++;
		#endif
	};

	~ListLockable()
	{
		#ifdef LOGGING
		Logger::counter_list_lockable--;
		#endif
	};

	void Add(T& _element)
	{
		boost::unique_lock<boost::shared_mutex> lock(mutex);
		list.push_back(_element);
		++number;
	}

	void Remove(T& _element)
	{
		boost::unique_lock<boost::shared_mutex> lock(mutex);
		list.remove(_element);
		--number;
	}

	void Clear()
	{
		boost::unique_lock<boost::shared_mutex> lock(mutex);
		list.clear();
		number = 0;
	}

private:
	ListLockable(const ListLockable<T>& _other);
	ListLockable& operator=(const ListLockable<T>* _other);
};

#endif