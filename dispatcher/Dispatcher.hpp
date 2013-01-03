/*
	Dispatcher
	Copyright (c) 2012 Russell Bewley

	Dispatcher is free software released under the MIT License
	(http://www.opensource.org/licenses/mit-license.php)
*/
/*!
	\file Dispatcher.hpp 
	\brief Header file for the Dispatcher class.
*/

#ifndef DISPATCHER_HPP_INCLUDED
#define DISPATCHER_HPP_INCLUDED

#include "Dispatchables.hpp"

/*!
	The Dispatcher class maintains a queue of tasks that 
	need to be executed, and creates a worker thread to 
	monitor the queue, and to execute tasks that are 
	pushed into the queue.

	\warning Attempting to dispatch tasks that invoke 
	methods on the same Dispatcher will likely cause
	a deadlock; however, tasks that dispatch other
	tasks with the same dispatcher will work fine.

	\warning It is possible to enter a situation where 
	the worker thread goes to sleep at the same time a 
	new task is dispatched. In this situation, the thread
	may not wake up for a long time, so it is important to
	tune the timeout value to whatever tolerance you need. 
	This usually does not happen. This is just a fallback 
	in case it does. The default timeout period is 500 
	milliseconds.

	\note if the dispatcher is already running, calling 
	start() again will wake the worker thread. you may do
	this after dispatching tasks to ensure that the worker 
	thread isn't sleeping while there is work to be done.
*/
class Dispatcher : boost::noncopyable {
public:	
	typedef DispatchableFunction Task; // this is chosen as the default 
	                                   // Dispatchable for backwards compatibility
	typedef DispatchablePtr TaskPtr;
	typedef boost::posix_time::time_duration time_duration;

	//! construct a Dispatcher that must be started manually
	Dispatcher();

	//! construct a Dispatcher and optionally start it immediately.
	explicit Dispatcher(bool startImmediately);

	/*!
		construct a Dispatcher and optionally start it immediately.
		also specify the maximum amount of time that the worker thread
		should wait for a new task to be dispatched before waking up.
	*/
	explicit Dispatcher(bool startImmediately, const time_duration& waitTimeout);

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
	void dispatch(TaskPtr task);

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