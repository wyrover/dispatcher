/*
	Dispatcher
	Copyright (c) 2013 Russell Bewley

	http://github.com/rbewley4/dispatcher

	Dispatcher is free software released under the MIT License
	(http://www.opensource.org/licenses/mit-license.php)
*/
/*!
	\file DispatchablesTests.cpp 
	\brief Unit tests for the Dispatchable class and subclasses. 
*/

#include "vlt.hpp"
#include "Dispatcher.hpp"
#include "Dispatchables.hpp"
#include <boost/limits.hpp>
#include <boost/thread.hpp>
#include <vector>

namespace {
	
	struct TestFixture {
		TestFixture()
		{
			reset();
		}

		void reset()
		{
			counter = 0;
		}

		void increment()
		{
			++counter;
		}

		int counter;
	};

	TEST(Dispatchables, OneShotTask)
	{
		TestFixture f;
		Dispatcher d(true);
		DispatchablePtr task;

		task = DispatchablePtr(new DispatchableFunction(boost::bind(&TestFixture::increment, &f)));
		d.dispatch(task);

		// wait until tasks are done executing
		while(f.counter < 1);

		TEST_EQUALS(f.counter, 1);
	}

	TEST(Dispatchables, recurringTask)
	{
		TestFixture f;
		Dispatcher d(true);
		DispatchablePtr task;

		int period_ms = 100;
		boost::posix_time::time_duration period = boost::posix_time::milliseconds(period_ms);		
		task = DispatchablePtr(new RecurringDispatchableFunction(boost::bind(&TestFixture::increment, &f),period));

		d.dispatch(task);

		// sleep for triple the period to ensure it was at least called twice
		// note: this is dependent on timer resolution. the goal is just to
		// ensure that the task recurs approximately within in the period
		// requested.
		boost::this_thread::sleep(boost::posix_time::milliseconds(period_ms*3));

		TEST_GREATER_THAN(f.counter, 2);
	}

	TEST(Dispatchables, iterativeTask)
	{
		TestFixture f;
		Dispatcher d(true);
		DispatchablePtr task;

		task = DispatchablePtr(new IterativeDispatchableFunction(boost::bind(&TestFixture::increment, &f),100));
		d.dispatch(task);

		// wait until tasks are done executing
		while(f.counter < 100);

		TEST_EQUALS(f.counter, 100);
	}
} //namespace