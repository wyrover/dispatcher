/*
	Dispatcher
	Copyright (c) 2013 Russell Bewley

	Dispatcher is free software released under the MIT License
	(http://www.opensource.org/licenses/mit-license.php)
*/
/*!
	\file Dispatchable.hpp 
	\brief Header file for the Dispatchable interface and ptr type.
*/

#ifndef DISPATCHABLE_HPP_INCLUDED
#define DISPATCHABLE_HPP_INCLUDED

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

#endif //DISPATCHABLE_HPP_INCLUDED