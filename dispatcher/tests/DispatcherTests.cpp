/*
	Dispatcher
	Copyright (c) 2012 Russell Bewley

	http://github.com/rbewley4/dispatcher

	Dispatcher is free software released under the MIT License
	(http://www.opensource.org/licenses/mit-license.php)
*/
/*!
	\file DispatcherTests.cpp 
	\brief Unit tests for the Dispatcher class. 
*/

#include "vlt.hpp"
#include "Dispatcher.hpp"
#include "Dispatchables.hpp"
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
			taskInvoked = false;
			a = 0;
		}

		void myTask(int a)
		{
			taskInvoked = true;
			this->a = a;
		}

		void myRentrantTask(Dispatcher& d, int a)
		{
			DispatchablePtr task(new DispatchableFunction(boost::bind(&TestFixture::myTask, this, a)));
			d.dispatch(task);
		}

		bool taskInvoked;
		int a;
	};

	TEST(Dispatcher, Construct)
	{
		Dispatcher d1;
		Dispatcher d2(false);
	}

	TEST(Dispatcher, StartAndStop)
	{
		Dispatcher d;

		d.start();
		while(!d.isRunning()) {}; //wait until started
		TEST_EQUALS(d.isRunning(), true);

		d.stop();
		while(d.isRunning()) {}; //wait until stopped
		TEST_EQUALS(d.isRunning(), false);
	}

	TEST(Dispatcher, StartImmediately)
	{
		Dispatcher d(true);
		while(!d.isRunning()) {}; //wait until started
		TEST_EQUALS(d.isRunning(), true);
	}

	TEST(Dispatcher, DispatchTask)
	{
		TestFixture f;
		Dispatcher d(true);

		DispatchablePtr task(new DispatchableFunction(boost::bind(&TestFixture::myTask, &f, 10)));
		d.dispatch(task);

		while(!f.taskInvoked) {}; // wait for task to be invoked
		TEST_EQUALS(f.taskInvoked, true);
		TEST_EQUALS(f.a, 10);
	}

	TEST(Dispatcher, DispatchReentrantTask)
	{
		TestFixture f;
		Dispatcher d(true);

		DispatchablePtr task(new DispatchableFunction(boost::bind(&TestFixture::myRentrantTask, &f, boost::ref(d), 10)));
		d.dispatch(task);

		while(!f.taskInvoked) {}; // wait for task to be invoked
		TEST_EQUALS(f.taskInvoked, true);
		TEST_EQUALS(f.a, 10);
	}

	TEST(Dispatcher, Size)
	{
		TestFixture f;
		Dispatcher d;
		DispatchablePtr task;

		TEST_EQUALS(d.isRunning(), false);

		const int NUM_TASKS = 1000;

		for(int i = 0; i < NUM_TASKS; ++i)
		{
			// add invalid task
			task = DispatchablePtr();
			d.dispatch(task);
		}

		TEST_EQUALS(d.size(), 0);

		for(int i = 0; i < NUM_TASKS; ++i)
		{
			// add valid task
			task = DispatchablePtr(new DispatchableFunction(boost::bind(&TestFixture::myTask, &f, 10)));
			d.dispatch(task);
		}

		TEST_EQUALS(d.size(), static_cast<size_t>(NUM_TASKS));
	}

	TEST(Dispatcher, Empty)
	{
		TestFixture f;
		Dispatcher d;
		DispatchablePtr task;

		TEST_EQUALS(d.isRunning(), false);
		TEST_EQUALS(d.empty(), true);

		// add invalid task (dispatch is ignored)
		task = DispatchablePtr();
		d.dispatch(task);

		TEST_EQUALS(d.empty(), true);

		// add valid task
		task = DispatchablePtr(new DispatchableFunction(boost::bind(&TestFixture::myTask, &f, 10)));
		d.dispatch(task);

		TEST_EQUALS(d.empty(), false);
	}
		
	TEST(Dispatcher, HeavyWorkLoad)
	{
		const int NUM_TASKS = 1000;

		std::vector<TestFixture> f(NUM_TASKS);
		Dispatcher d(true);

		for(int i = 0; i < NUM_TASKS; ++i)
		{
			DispatchablePtr task(new DispatchableFunction(boost::bind(&TestFixture::myTask, &f[i], i)));
			d.dispatch(task);
		}

		for(int i = 0; i < NUM_TASKS; ++i)
		{
			while(!f[i].taskInvoked) {}; // wait for task to be invoked
			TEST_EQUALS(f[i].taskInvoked, true);
			TEST_EQUALS(f[i].a, i);
		}		
	}

	TEST(Dispatcher, HeavierWorkLoad)
	{
		const int NUM_TASKS = 10000;

		std::vector<TestFixture> f(NUM_TASKS);
		Dispatcher d(true);

		for(int i = 0; i < NUM_TASKS; ++i)
		{
			DispatchablePtr task(new DispatchableFunction(boost::bind(&TestFixture::myTask, &f[i], i)));
			d.dispatch(task);
		}

		for(int i = 0; i < NUM_TASKS; ++i)
		{
			while(!f[i].taskInvoked) {}; // wait for task to be invoked
			TEST_EQUALS(f[i].taskInvoked, true);
			TEST_EQUALS(f[i].a, i);
		}		
	}

	TEST(Dispatcher, HeaviestWorkLoad)
	{
		const int NUM_TASKS = 100000;

		std::vector<TestFixture> f(NUM_TASKS);
		Dispatcher d(true);

		for(int i = 0; i < NUM_TASKS; ++i)
		{
			DispatchablePtr task(new DispatchableFunction(boost::bind(&TestFixture::myTask, &f[i], i)));
			d.dispatch(task);
		}

		for(int i = 0; i < NUM_TASKS; ++i)
		{
			while(!f[i].taskInvoked) {}; // wait for task to be invoked
			TEST_EQUALS(f[i].taskInvoked, true);
			TEST_EQUALS(f[i].a, i);
		}		
	}

	TEST(Dispatcher, NullTaskPtr)
	{
		Dispatcher d(true);
		DispatchablePtr nullTaskPtr;
		d.dispatch(nullTaskPtr);
	}

	TEST(Dispatcher, StartStopStress)
	{
		Dispatcher d;

		const int NUM_TESTS = 1000;

		for(int i = 0; i < NUM_TESTS; ++i)
		{
			d.start();
			while(!d.isRunning()) {}; //wait until started
			TEST_EQUALS(d.isRunning(), true);

			d.stop();
			while(d.isRunning()) {}; //wait until stopped
			TEST_EQUALS(d.isRunning(), false);
		}
	}

	TEST(Dispatcher, StartStopThrash)
	{
		Dispatcher d;

		const int NUM_TESTS = 1000;

		for(int i = 0; i < NUM_TESTS; ++i)
		{
			d.start();
			d.stop();
		}

		while(d.isRunning()) {}; //wait until stopped
		TEST_EQUALS(d.isRunning(), false);
	}

	TEST(Dispatcher, StartStopStressMultiThread)
	{
		Dispatcher testee;
		Dispatcher worker(true);
		DispatchablePtr task;
		
		const int NUM_TESTS = 1000;

		for(int i = 0; i < NUM_TESTS; ++i)
		{
			// start testee
			task = DispatchablePtr(new DispatchableFunction(boost::bind(&Dispatcher::start, &testee)));
			worker.dispatch(task);
			while(!testee.isRunning()) {}; //wait until started
			TEST_EQUALS(testee.isRunning(), true);

			// stop testee
			task = DispatchablePtr(new DispatchableFunction(boost::bind(&Dispatcher::stop, &testee)));
			worker.dispatch(task);
			while(testee.isRunning()) {}; //wait until stopped
			TEST_EQUALS(testee.isRunning(), false);
		}
	}

	TEST(Dispatcher, StartStopThrashMultiThread)
	{
		Dispatcher testee;
		Dispatcher worker(true);
		DispatchablePtr task;
		
		const int NUM_TESTS = 1000;

		for(int i = 0; i < NUM_TESTS; ++i)
		{
			// start testee
			task = DispatchablePtr(new DispatchableFunction(boost::bind(&Dispatcher::start, &testee)));
			worker.dispatch(task);

			// stop testee
			task = DispatchablePtr(new DispatchableFunction(boost::bind(&Dispatcher::stop, &testee)));
			worker.dispatch(task);
		}
	}

} //namespace