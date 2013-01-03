/*
	Dispatcher
	Copyright (c) 2013 Russell Bewley

	Dispatcher is free software released under the MIT License
	(http://www.opensource.org/licenses/mit-license.php)
*/
/*!
	\file Dispatchables.cpp 
	\brief Implementation of the Dispatchable class and subclasses. 
*/

#include "Dispatchables.hpp"
#include <boost/bind.hpp>

namespace {
	//! calls a void() function and returns false
	bool CallableAdapter(boost::function0<void> func)
	{
		func();
		return(false);
	}
}

Dispatchable::Dispatchable()
	: func_(), recurring_(false), period_(), last_run_()
{
}

Dispatchable::Dispatchable(Callable func, bool dummy)
	: func_(func), recurring_(false), period_(), last_run_()
{
}

Dispatchable::Dispatchable(Callable func, const time_duration& period)
	: func_(func), recurring_(true), period_(period), last_run_()
{
}

Dispatchable::Dispatchable(boost::function0<void> func)
	: func_(Callable(boost::bind(CallableAdapter, func))), recurring_(false), period_(), last_run_()
{
}

Dispatchable::Dispatchable(const Dispatchable& rhs)
	: func_(rhs.func_), 
	recurring_(rhs.recurring_), 
	period_(rhs.period_), 
	last_run_(rhs.last_run_)
{
}

Dispatchable& Dispatchable::operator=(const Dispatchable& rhs)
{
	func_ = rhs.func_;
	recurring_ = rhs.recurring_;
	period_ = rhs.period_;
	last_run_ = rhs.last_run_;
	return(*this);
}

Dispatchable::~Dispatchable()
{
}

bool Dispatchable::isValid() const
{
	return(!func_.empty());
}

void Dispatchable::clear()
{
	func_.clear();
	recurring_ = false;
	period_ = time_duration();
	last_run_ = ptime();
}

bool Dispatchable::isRecurring() const
{
	return(recurring_);
}

bool Dispatchable::shouldRecur() const
{
	if(isRecurring())
	{
		ptime now = boost::posix_time::microsec_clock::local_time();
		time_duration elapsed = now - last_run_;
		return(elapsed >= period_);
	}
	else
	{
		return(false);
	}
}

bool Dispatchable::run()
{
	bool recur = false;

	if(isValid())
	{
		recur = func_();
		if(isRecurring())
		{
			last_run_ = boost::posix_time::microsec_clock::local_time();
		}
	}

	return(recur);
}
