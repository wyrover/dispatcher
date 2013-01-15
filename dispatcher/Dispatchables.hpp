/*
	Dispatcher
	Copyright (c) 2013 Russell Bewley

	http://github.com/rbewley4/dispatcher

	Dispatcher is free software released under the MIT License
	(http://www.opensource.org/licenses/mit-license.php)
*/
/*!
	\file Dispatchables.hpp 
	\brief Header file for the Dispatchable class, and subclasses.
*/

#ifndef DISPATCHABLES_HPP_INCLUDED
#define DISPATCHABLES_HPP_INCLUDED

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/utility.hpp>

/*!
	The Dispatchable interface abstracts a task that can be run
	via the Dispatcher. It also allows for task recurrence.
*/
class Dispatchable {
public:
	virtual ~Dispatchable() {};

	//! execute the task
	virtual void run() = 0;

	/*!
		if the task is recurring, it will be added back
		to the queue so that it can be evaluated in the future.

		\return true if the task is recurring
	*/
	virtual bool isRecurring() = 0;

	/*!
		\return true if it is time for the task to execute 
		\note if both shouldExecute() and isRecurring() return
		false, then this task will never be executed.
	*/
	virtual bool shouldExecute() = 0;
};


//! a reference-counted pointer to a Dispatchable object
typedef boost::shared_ptr<Dispatchable> DispatchablePtr;


/*!
	The DispatchableFunction class is a wrapper to use
	a function object as a dispatchable task.
*/
class DispatchableFunction : public Dispatchable, boost::noncopyable {
public:
	typedef boost::function0<void> Callable;

	/*!
		Construct a DispatchableFunction object

		\param func the function to be run when this task is executed. 
	*/
	explicit DispatchableFunction(Callable func);

	//! Destroy a Dispatchable object
	virtual ~DispatchableFunction();

	//! \return true if the task is recurring
	virtual bool isRecurring();

	//! \return true if it is time for the task to execute 
	virtual bool shouldExecute();

	//! Execute the task.
	virtual void run();

private:
	Callable func_;
};


/*!
	The RecurringDispatchableFunction class is a wrapper
	to use a function object as a dispatchable task, and 
	allows for recurrence.
*/
class RecurringDispatchableFunction : public DispatchableFunction {
public:
	typedef DispatchableFunction::Callable Callable;
	typedef boost::posix_time::time_duration time_duration;
	typedef boost::posix_time::ptime ptime;

	/*!
		Construct a RecurringDispatchableFunction object

		\param func the function to be run when this task is executed. 
		\param period the minimum amount of to wait before executing the task again.
		\note this task will recur forever
	*/
	explicit RecurringDispatchableFunction(Callable func, const time_duration& period);

	//! Destroy a Dispatchable object
	virtual ~RecurringDispatchableFunction();

	//! \return true if the task is recurring
	virtual bool isRecurring();

	//! \return true if it is time for the task to execute 
	virtual bool shouldExecute();

	//! Execute the task.
	virtual void run();

private:
	Callable func_;
	time_duration period_;
	ptime last_run_;
};

/*!
	The IterativeDispatchableFunction class is a wrapper to use
	a function object as a dispatchable task, and allows for 
	recurrence.
*/
class IterativeDispatchableFunction : public DispatchableFunction {
public:
	typedef DispatchableFunction::Callable Callable;

	/*!
		Construct a IterativeDispatchableFunction object

		\param func the function to be run when this task is executed.
		\param times_to_repeat the number of times for this task to recur
	*/
	explicit IterativeDispatchableFunction(Callable func, size_t times_to_repeat);

	//! Destroy a Dispatchable object
	virtual ~IterativeDispatchableFunction();

	//! \return true if the task is recurring
	virtual bool isRecurring();

	//! \return true if it is time for the task to execute 
	virtual bool shouldExecute();

	//! Execute the task.
	virtual void run();

private:
	Callable func_;
	size_t times_to_repeat_;
	size_t count_;
};

#endif //DISPATCHABLES_HPP_INCLUDED