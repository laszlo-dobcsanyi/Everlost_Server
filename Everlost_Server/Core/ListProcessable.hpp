#ifndef CORE_LIST_PROCESSABLE_HPP
#define CORE_LIST_PROCESSABLE_HPP

#include <boost\foreach.hpp>

#include "Core\ListLockable.hpp"

template<class T> class ListProcessable_Callback
{
public:
	inline ListProcessable_Callback() { };
	inline ~ListProcessable_Callback() { };

	virtual void ProcessAdding(T _current) = 0;
	virtual void ProcessRemoving(T _current) = 0;

private:
	ListProcessable_Callback(const ListProcessable_Callback<T>& _other);
	ListProcessable_Callback& operator=(const ListProcessable_Callback<T>* _other);
};


template<class T> class ListProcessable
{
public:
	ListLockable<T> data;
	ListLockable<T> adding;
	ListLockable<T> removing;

	ListProcessable()
	{
		#ifdef LOGGING
		Logger::counter_list_processable++;
		#endif
	};
	
	~ListProcessable()
	{
		#ifdef LOGGING
		Logger::counter_list_processable--;
		#endif
	};

	void Add(T _element) { adding.Add(_element); }
	void Remove(T _element) { removing.Add(_element); }

	void Process_Adding()
	{
		if (0 < adding.number)
		{
			boost::unique_lock<boost::shared_mutex> adding_lock(adding.mutex);
			boost::unique_lock<boost::shared_mutex> data_lock(data.mutex);

			BOOST_FOREACH(T _elem, adding.list)
			{
				data.list.push_back(_elem);
			}
			data.number += adding.number;
			data_lock.unlock();

			adding.list.clear();
			adding.number = 0;
			adding_lock.unlock();
		}
	}

	void Process_Adding(ListProcessable_Callback<T>* _object)
	{
		if (0 < adding.number)
		{
			boost::unique_lock<boost::shared_mutex> adding_lock(adding.mutex);
			boost::unique_lock<boost::shared_mutex> data_lock(data.mutex);

			BOOST_FOREACH(T _elem, adding.list)
			{
				data.list.push_back(_elem);
				_object->ProcessAdding(_elem);
			}
			data.number += adding.number;
			data_lock.unlock();

			adding.list.clear();
			adding.number = 0;
			adding_lock.unlock();
		}
	}

	void Process_Removing()
	{
		if (0 < removing.number)
		{
			boost::unique_lock<boost::shared_mutex> removing_lock(removing.mutex);
			boost::unique_lock<boost::shared_mutex> data_lock(data.mutex);

			BOOST_FOREACH(T _elem, removing.list)
			{
				data.list.remove(_elem);
			}
			data.number -= removing.number;
			data_lock.unlock();

			removing.list.clear();
			removing.number = 0;
			removing_lock.unlock();
		}	
	}

	void Process_Removing(ListProcessable_Callback<T>* _object)
	{
		if (0 < removing.number)
		{
			boost::unique_lock<boost::shared_mutex> removing_lock(removing.mutex);
			boost::unique_lock<boost::shared_mutex> data_lock(data.mutex);

			BOOST_FOREACH(T _elem, removing.list)
			{
				_object->ProcessRemoving(_elem);
				data.list.remove(_elem);
			}
			data.number -= removing.number;
			data_lock.unlock();

			removing.list.clear();
			removing.number = 0;
			removing_lock.unlock();
		}	
	}

private:
	ListProcessable(const ListProcessable<T>& _other);
	ListProcessable& operator=(const ListProcessable<T>* _other);
};

#endif