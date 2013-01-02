/*
	Dispatcher
	Copyright (c) 2012 Russell Bewley

	Dispatcher is free software released under the MIT License
	(http://www.opensource.org/licenses/mit-license.php)
*/
/*!
	\file DispatcherTests.cpp 
	\brief Unit tests for the Dispatcher class. 
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
			taskInvoked = false;
			a = 0;
			recurCounter = 0;
		}

		void myTask(int a)
		{
			taskInvoked = true;
			this->a = a;
		}

		void myRentrantTask(Dispatcher& d, int a)
		{
			Dispatcher::TaskPtr task(new Dispatcher::Task(boost::bind(&TestFixture::myTask, this, a)));
			d.dispatch(task);
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

		bool taskInvoked;
		int a;
		int recurCounter;
	};

	TEST(Dispatcher, Construct)
	{
		Dispatcher d1;
		Dispatcher d2(false);
		Dispatcher d3(false, boost::posix_time::milliseconds(5000));
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

		Dispatcher::TaskPtr task(new Dispatcher::Task(boost::bind(&TestFixture::myTask, &f, 10)));
		d.dispatch(task);

		while(!f.taskInvoked) {}; // wait for task to be invoked
		TEST_EQUALS(f.taskInvoked, true);
		TEST_EQUALS(f.a, 10);
	}

	TEST(Dispatcher, DispatchReentrantTask)
	{
		TestFixture f;
		Dispatcher d(true);

		Dispatcher::TaskPtr task(new Dispatcher::Task(boost::bind(&TestFixture::myRentrantTask, &f, boost::ref(d), 10)));
		d.dispatch(task);

		while(!f.taskInvoked) {}; // wait for task to be invoked
		TEST_EQUALS(f.taskInvoked, true);
		TEST_EQUALS(f.a, 10);
	}

	TEST(Dispatcher, Size)
	{
		TestFixture f;
		Dispatcher d;
		Dispatcher::TaskPtr task;

		TEST_EQUALS(d.isRunning(), false);

		const int NUM_TASKS = 1000;

		for(int i = 0; i < NUM_TASKS; ++i)
		{
			// add invalid task
			task = Dispatcher::TaskPtr(new Dispatcher::Task());
			d.dispatch(task);
		}

		TEST_EQUALS(d.size(), 0);

		for(int i = 0; i < NUM_TASKS; ++i)
		{
			// add valid task
			task = Dispatcher::TaskPtr(new Dispatcher::Task(boost::bind(&TestFixture::myTask, &f, 10)));
			d.dispatch(task);
		}

		TEST_EQUALS(d.size(), static_cast<size_t>(NUM_TASKS));
	}

	TEST(Dispatcher, Empty)
	{
		TestFixture f;
		Dispatcher d;
		Dispatcher::TaskPtr task;

		TEST_EQUALS(d.isRunning(), false);
		TEST_EQUALS(d.empty(), true);

		// add invalid task (dispatch is ignored)
		task = Dispatcher::TaskPtr(new Dispatcher::Task());
		d.dispatch(task);

		TEST_EQUALS(d.empty(), true);

		// add valid task
		task = Dispatcher::TaskPtr(new Dispatcher::Task(boost::bind(&TestFixture::myTask, &f, 10)));
		d.dispatch(task);

		TEST_EQUALS(d.empty(), false);
	}
	
	TEST(Dispatcher, OneShotTask)
	{
		TestFixture f;
		Dispatcher d(true);
		Dispatcher::TaskPtr task;

		task = Dispatcher::TaskPtr(new Dispatcher::Task(boost::bind(&TestFixture::oneshotTask, &f),true));
		d.dispatch(task);

		// wait until tasks are done executing
		while(f.recurCounter < 1);

		TEST_EQUALS(f.recurCounter, 1);
	}

	TEST(Dispatcher, recurTenTimes)
	{
		TestFixture f;
		Dispatcher d(true);
		Dispatcher::TaskPtr task;

		task = Dispatcher::TaskPtr(new Dispatcher::Task(boost::bind(&TestFixture::recurTenTimes, &f),true));
		d.dispatch(task);

		// wait until tasks are done executing
		while(f.recurCounter < 10);

		TEST_EQUALS(f.recurCounter, 10);
	}
	
	TEST(Dispatcher, HeavyWorkLoad)
	{
		const int NUM_TASKS = 1000;

		std::vector<TestFixture> f(NUM_TASKS);
		Dispatcher d(true);

		for(int i = 0; i < NUM_TASKS; ++i)
		{
			Dispatcher::TaskPtr task(new Dispatcher::Task(boost::bind(&TestFixture::myTask, &f[i], i)));
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
			Dispatcher::TaskPtr task(new Dispatcher::Task(boost::bind(&TestFixture::myTask, &f[i], i)));
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
			Dispatcher::TaskPtr task(new Dispatcher::Task(boost::bind(&TestFixture::myTask, &f[i], i)));
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
		Dispatcher::TaskPtr nullTaskPtr;
		d.dispatch(nullTaskPtr);
	}

	TEST(Dispatcher, NullTask)
	{
		Dispatcher d(true);
		Dispatcher::TaskPtr nullTask(new Dispatcher::Task());
		d.dispatch(nullTask);
	}

	TEST(Dispatcher, ClearedTask)
	{
		TestFixture f;
		Dispatcher d(true);
		Dispatcher::TaskPtr clearedTask(new Dispatcher::Task(boost::bind(&TestFixture::myTask, &f, 10)));
		clearedTask->clear();
		d.dispatch(clearedTask);
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
		Dispatcher::TaskPtr task;
		
		const int NUM_TESTS = 1000;

		for(int i = 0; i < NUM_TESTS; ++i)
		{
			// start testee
			task = Dispatcher::TaskPtr(new Dispatcher::Task(boost::bind(&Dispatcher::start, &testee)));
			worker.dispatch(task);
			while(!testee.isRunning()) {}; //wait until started
			TEST_EQUALS(testee.isRunning(), true);

			// stop testee
			task = Dispatcher::TaskPtr(new Dispatcher::Task(boost::bind(&Dispatcher::stop, &testee)));
			worker.dispatch(task);
			while(testee.isRunning()) {}; //wait until stopped
			TEST_EQUALS(testee.isRunning(), false);
		}
	}

	TEST(Dispatcher, StartStopThrashMultiThread)
	{
		Dispatcher testee;
		Dispatcher worker(true);
		Dispatcher::TaskPtr task;
		
		const int NUM_TESTS = 1000;

		for(int i = 0; i < NUM_TESTS; ++i)
		{
			// start testee
			task = Dispatcher::TaskPtr(new Dispatcher::Task(boost::bind(&Dispatcher::start, &testee)));
			worker.dispatch(task);

			// stop testee
			task = Dispatcher::TaskPtr(new Dispatcher::Task(boost::bind(&Dispatcher::stop, &testee)));
			worker.dispatch(task);
		}
	}

} //namespace