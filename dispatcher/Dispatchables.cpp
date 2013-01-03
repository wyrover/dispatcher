/*
	Dispatcher
	Copyright (c) 2013 Russell Bewley

	Dispatcher is free software released under the MIT License
	(http://www.opensource.org/licenses/mit-license.php)
*/
/*!
	\file DispatchableFunctions.cpp 
	\brief Implementation of the DispatchableFunction class and subclasses. 
*/

#include "Dispatchables.hpp"
#include <boost/bind.hpp>

/*
 * DispatchableFunction class implementation
 */
DispatchableFunction::DispatchableFunction(Callable func)
	: Dispatchable(), func_(func)
{
}

DispatchableFunction::~DispatchableFunction()
{
}

bool DispatchableFunction::isRecurring()
{
	return(false);
}

bool DispatchableFunction::shouldExecute()
{
	return(true);
}

void DispatchableFunction::run()
{
	if(func_)
	{
		func_();
	}
}


/*
 * RecurringDispatchableFunction class implementation
 */
RecurringDispatchableFunction::RecurringDispatchableFunction(Callable func, const time_duration& period)
	: DispatchableFunction(func), period_(period), last_run_()
{
}

RecurringDispatchableFunction::~RecurringDispatchableFunction()
{
}

bool RecurringDispatchableFunction::isRecurring()
{
	return(true);
}

bool RecurringDispatchableFunction::shouldExecute()
{
	ptime now = boost::posix_time::microsec_clock::local_time();
	time_duration elapsed = now - last_run_;
	return(elapsed >= period_);
}

void RecurringDispatchableFunction::run()
{
	DispatchableFunction::run();
}