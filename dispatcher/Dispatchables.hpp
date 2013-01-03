/*
	Dispatcher
	Copyright (c) 2013 Russell Bewley

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

#endif //DISPATCHABLES_HPP_INCLUDED