/*
	Dispatcher
	Copyright (c) 2012 Russell Bewley

	http://github.com/rbewley4/dispatcher

	Dispatcher is free software released under the MIT License
	(http://www.opensource.org/licenses/mit-license.php)
*/
/*!
	\file Dispatcher.hpp 
	\brief Header file for the Dispatcher class.
*/

#ifndef DISPATCHER_HPP_INCLUDED
#define DISPATCHER_HPP_INCLUDED

#include "Dispatchable.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

//! a reference-counted pointer to a Dispatchable object
typedef boost::shared_ptr<Dispatchable> DispatchablePtr;

/*!
	The Dispatcher class maintains a queue of tasks that 
	need to be executed, and creates a worker thread to 
	monitor the queue, and to execute tasks that are 
	pushed into the queue.

	\warning Attempting to dispatch tasks that invoke 
	methods on the same Dispatcher will likely cause
	a deadlock; however, tasks that dispatch other
	tasks with the same dispatcher will work fine.

	\note If you need real-time performance, you should
	disable the worker thread's wait behavior. This will
	prevent it from going idle.
*/
class Dispatcher : private boost::noncopyable {
public:	

	//! construct a Dispatcher that must be started manually
	Dispatcher();

	//! construct a Dispatcher and optionally start it immediately.
	explicit Dispatcher(bool startImmediately);

	/*!
		construct a Dispatcher and optionally start it immediately.
		also choose to disable the worker thread's wait behavior.
		This will improve performance so that you can use it in
		real-time applications.
	*/
	explicit Dispatcher(bool startImmediately, bool disableWait);

	//! destroying the Dispatcher stops the worker thread.
	~Dispatcher();

	/*!
		starts the dispatch worker thread. 
		
		\note if the dispatcher is already running, calling start() again 
		will wake the worker thread.
	*/
	void start();

	//! stops the dispatch worker thread
	void stop();

	//! checks to see if the dispatch worker thread is running
	bool isRunning();

	//! adds a task to the dispatch queue
	void dispatch(DispatchablePtr task);

	//! clear all tasks from the dispatch queue
	void clear();

	//! get the number of tasks in the queue
	size_t size() ;

	//! check if size() == 0
	bool empty();

private:
	class Impl;
	boost::shared_ptr<Impl> impl_;
};

#endif //DISPATCHER_HPP_INCLUDED