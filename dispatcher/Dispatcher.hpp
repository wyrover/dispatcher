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
#include <boost/date_time/posix_time/posix_time_types.hpp>

/*!
	The Dispatchable class is the context for an executable task.
	It maintains a function object, which is the task itself, plus
	timing information for recurring tasks.
*/
class Dispatchable {
public:
	typedef boost::function0<bool> Callable;
	typedef boost::posix_time::time_duration time_duration;
	typedef boost::posix_time::ptime ptime;

	//! Construct an invalid Dispatchable object
	Dispatchable();

	/*!
		Construct a Dispatchable object

		\param func the function to be run when this task is executed. 
		the return value allows the task to recur.
		\param dummy unused. it only exists to resolve ambiguity between constructors
	*/
	explicit Dispatchable(Callable func, bool dummy);

	/*!
		Construct a Dispatchable object that will recur

		\param func the function to be run when this task is executed. 
		\param period the minimum amount of time to wait before executing the task again
	*/
	explicit Dispatchable(Callable func, const time_duration& period);

	/*!
		Construct a Dispatchable object

		\param func the function to be run when this task is executed. 

		\note this constructor exists for backwards compatibility
	*/
	explicit Dispatchable(boost::function0<void> func);

	//! Copy a Dispatchable object
	Dispatchable(const Dispatchable& rhs);

	//! Destroy a Dispatchable object
	~Dispatchable();

	//! Assignment operator
	Dispatchable& operator=(const Dispatchable& rhs);

	//! \return true if function object is not null
	bool isValid() const;

	//! make this Dispatchable object invalid
	void clear();

	//! \return true if the task is recurring
	bool isRecurring() const;

	//! \return true if it is time for the task to recur
	bool shouldRecur() const;

	/*!
		Execute the task.
		\return true if the task should be executed again, false otherwise.
	*/
	bool run();

private:
	Callable func_;
	bool recurring_;
	time_duration period_;
	ptime last_run_;
};


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
*/
class Dispatcher : boost::noncopyable {
public:	
	typedef Dispatchable Task;
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

		\note: It is possible to enter a situation where the worker thread
		is going to sleep at the same time a new task is dispatched. In
		this situation, the thread may not wake up for a long time, so
		it is important to tune the timeout value to whatever tolerance you
		need. This usually does not happen. This is just a fallback in case
		it does. The default timeout period is 500 milliseconds
	*/
	explicit Dispatcher(bool startImmediately, const time_duration& waitTimeout);

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

	//! check if size() == 0
	bool empty();

private:
	class Impl;
	boost::shared_ptr<Impl> impl_;
};

#endif //DISPATCHER_HPP_INCLUDED