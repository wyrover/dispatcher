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

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

/*!
	The Dispatcher class maintains a queue of tasks that 
	need to be executed, and creates a worker thread to 
	monitor the queue, and to execute tasks that are 
	pushed into the queue.

	\warning Attempting to dispatch tasks that invoke 
	methods on the same Dispatcher will likely cause
	a deadlock; however, tasks that dispatch other
	tasks with the same dispatcher will work fine.
*/
class Dispatcher : boost::noncopyable {
public:	
	typedef boost::function0<void> Task;
	typedef boost::shared_ptr<Task> TaskPtr;

	//! dispatch worker thread must be started manually
	Dispatcher();

	//! optionally start the dispatch worker thread immediately
	explicit Dispatcher(bool startImmediately);

	//! destroying the Dispatcher stops the worker thread.
	~Dispatcher();

	//! starts the dispatch worker thread
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

private:
	class Impl;
	boost::shared_ptr<Impl> impl_;
};

#endif //DISPATCHER_HPP_INCLUDED