/*
	Dispatcher
	Copyright (c) 2013 Russell Bewley

	Dispatcher is free software released under the MIT License
	(http://www.opensource.org/licenses/mit-license.php)
*/
/*!
	\file DispatchablesTests.cpp 
	\brief Unit tests for the Dispatchable class and subclasses. 
*/

#include "vlt.hpp"
#include "Dispatcher.hpp"
#include <boost/limits.hpp>
#include <vector>

namespace {
	
	struct TestFixture {
		TestFixture()
		{
			reset();
		}

		void reset()
		{
			recurCounter = 0;
		}

		bool oneshotTask()
		{
			++recurCounter;
			return false;
		}

		bool recurTenTimes()
		{
			return (++recurCounter < 10);
		}

		int recurCounter;
	};

	TEST(Dispatchables, OneShotTask)
	{
		TestFixture f;
		Dispatcher d(true);
		DispatchablePtr task;

		task = DispatchablePtr(new DispatchableFunction(boost::bind(&TestFixture::oneshotTask, &f)));
		d.dispatch(task);

		// wait until tasks are done executing
		while(f.recurCounter < 1);

		TEST_EQUALS(f.recurCounter, 1);
	}

	TEST(Dispatchables, OneShotTaskBackwardsCompatibility)
	{
		TestFixture f;
		Dispatcher d(true);
		Dispatcher::TaskPtr task;

		task = Dispatcher::TaskPtr(new Dispatcher::Task(boost::bind(&TestFixture::oneshotTask, &f)));
		d.dispatch(task);

		// wait until tasks are done executing
		while(f.recurCounter < 1);

		TEST_EQUALS(f.recurCounter, 1);
	}

	TEST(Dispatchables, recurTenTimes)
	{
		TestFixture f;
		Dispatcher d(true);
		DispatchablePtr task;

		boost::posix_time::time_duration period = boost::posix_time::milliseconds(500);

		task = DispatchablePtr(new RecurringDispatchableFunction(boost::bind(&TestFixture::recurTenTimes, &f),period));
		d.dispatch(task);

		// wait until tasks are done executing
		while(f.recurCounter < 10);

		TEST_EQUALS(f.recurCounter, 10);
	}
} //namespace