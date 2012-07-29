/*
	Very Lite Test (VLT)
	Copyright (c) 2012 Russell Bewley

	VLT is free software released under the MIT License
	(http://www.opensource.org/licenses/mit-license.php)
*/
/*!
	\file vlt.cpp 
	\brief Implementation of the VLT testing framework. 
*/

#include "vlt.hpp"
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>

namespace test {
	using namespace internal;
	using namespace boost::posix_time;
	
	// class TestResult

	TestResult::TestResult( 
		TestPtr test,
		const std::string& expression, 
		const std::string& file, 
		unsigned line, 
		bool fatal, 
		bool success) :
		
		test_(test),
		expression_(expression),
		file_(file),
		line_(line),
		fatal_(fatal),
		success_(success)
	{
	}

	TestResult::TestResult(const TestResult& rhs)
	{
		*this = rhs;
	}

	TestResult::~TestResult()
	{
		test_.reset();
	}

	TestResult& TestResult::operator=(const TestResult& rhs)
	{
		test_ = rhs.test_;
		expression_ = rhs.expression_;
		file_ = rhs.file_;
		line_ = rhs.line_;
		fatal_ = rhs.fatal_;
		success_ = rhs.success_;
		return(*this);
	}

	TestPtr TestResult::getTest() const
	{
		return(test_);
	}

	const std::string& TestResult::getExpression() const
	{
		return(expression_);
	}

	const std::string& TestResult::getFile() const
	{
		return(file_);
	}

	unsigned TestResult::getLine() const
	{
		return(line_);
	}

	bool TestResult::isFatal() const
	{
		return(fatal_);
	}

	bool TestResult::isSuccess() const
	{
		return(success_);
	}

	bool TestResult::isFailure() const
	{
		return(!success_);
	}


	// class TestManager

	TestManager::TestManager() :
		tests_(), 
		stopTestsIndicator_(false), 
		currentTest_(),
		eventManager_()
	{
	}

	TestManager::~TestManager()
	{
		currentTest_.reset();

		// clear the test queue
		while(!tests_.empty())
		{
			tests_.pop();
		}
	}
	
	void TestManager::addTest(TestPtr test)
	{
		tests_.push(test);
		raise(eventManager_, &TestEventListener::onTestAdded, test);
	}

	void TestManager::runTests()
	{			
		raise(eventManager_, &TestEventListener::onTestingStarted);

		stopTestsIndicator_ = false;
		while(!tests_.empty() && !stopTestsIndicator_)
		{			
			currentTest_ = tests_.front();
			runOne(currentTest_);
			tests_.pop();
		}
		
		raise(eventManager_, &TestEventListener::onTestingStopped);
	}

	void TestManager::stopTests()
	{
		stopTestsIndicator_ = true;
	}

	TestPtr TestManager::getCurrentTest() const
	{
		return(currentTest_);
	}

	void TestManager::runOne(TestPtr test)
	{
		raise(eventManager_, &TestEventListener::onTestAboutToRun, test);

		test->execute();

		raise(eventManager_, &TestEventListener::onTestFinished, test);
	}

	void TestManager::reportTestResult(TestResultPtr result)
	{
		raise(eventManager_, &TestEventListener::onTestResult, result);

		if(result->isFailure() && result->isFatal())
		{
			stopTests();
		}
	}

	EventManager<TestEventListenerPtr>& TestManager::getEventManager()
	{
		return(eventManager_);
	}


	// class TestManagerFactory

	TestManagerPtr TestManagerFactory::getTestManager()
	{
		static TestManagerPtr testManager(new TestManager()); // singleton

		// Install a default event listener if there are no event listeners.
		if(testManager->getEventManager().getEventListeners().size() == 0)
		{
			TestEventListenerPtr defaultListener(new DefaultEventListener());
			testManager->getEventManager().add(defaultListener);
		}
		
		return testManager;
	}


	// class BaseTest

	BaseTest::BaseTest(const std::string& group, const std::string& name)
		: Test(), group_(group), name_(name)
	{		
	}

	BaseTest::BaseTest(const BaseTest& rhs)
		: Test(rhs), group_(rhs.group()), name_(rhs.name())
	{
	}

	BaseTest::~BaseTest()
	{
	}

	std::string BaseTest::group() const
	{
		return group_;
	}

	std::string BaseTest::name() const
	{
		return name_;
	}


	// class DefaultEventHandler

	/*!
		A hidden implementation class to hide the
		boost time dependencies from clients.
	*/
	class DefaultEventListener::Impl {
	public:
		Impl()
		{
		}

		~Impl()
		{
		}

		void printFailure(TestResultPtr result)
		{
			std::cerr << "!! Assertion failed in test case " << result->getTest()->group() 
					<< "::" << result->getTest()->name() << std::endl
					<< "   (" << result->getExpression() << ") in " << result->getFile() 
					<< ":" << result->getLine() << std::endl;
		}

		void printStatistics()
		{
			std::cout << std::endl;
			std::cout << "Test Results Summary:" << std::endl;
			std::cout << "=====================" << std::endl;		
			std::cout << "Passed Assertions: " << passedAssertCount << std::endl;
			std::cout << "Failed Assertions: " << failedAssertCount << std::endl;
			std::cout << "Total  Assertions: " << passedAssertCount + failedAssertCount << std::endl;
		}

		ptime testRunnerStarted;
		ptime testRunnerFinished;
		time_duration testRunnerDuration;
		ptime testStarted;
		ptime testFinished;
		time_duration testDuration;
		unsigned passedAssertCount;
		unsigned failedAssertCount;
	};


	DefaultEventListener::DefaultEventListener() :
		impl_(new DefaultEventListener::Impl())
	{
	}

	DefaultEventListener::~DefaultEventListener()
	{
		impl_.reset();
	}

	void DefaultEventListener::onTestAdded(TestPtr test)
	{
		// ignore
	}

	void DefaultEventListener::onTestingStarted()
	{
		impl_->passedAssertCount = 0;
		impl_->failedAssertCount = 0;

		impl_->testRunnerStarted = microsec_clock::local_time();
		std::cout << "TestManager started at " << to_simple_string(impl_->testRunnerStarted) << std::endl << std::endl;
	}

	void DefaultEventListener::onTestAboutToRun(TestPtr test)
	{
		std::cout << "Executing test " << test->group() << "::" << test->name() << std::endl;
		impl_->testStarted = microsec_clock::local_time();
	}

	void DefaultEventListener::onTestResult(TestResultPtr result)
	{
		if(result->isSuccess())
		{
			impl_->passedAssertCount += 1;
		}
		else
		{
			impl_->failedAssertCount += 1;
			impl_->printFailure(result);
		}
	}

	void DefaultEventListener::onTestFinished(TestPtr test)
	{
		impl_->testFinished = microsec_clock::local_time();
		impl_->testDuration = impl_->testFinished - impl_->testStarted;
		std::cout << "Test completed in " << to_simple_string(impl_->testDuration) << std::endl << std::endl;
	}

	void DefaultEventListener::onTestingStopped()
	{
		impl_->testRunnerFinished = microsec_clock::local_time();
		impl_->testRunnerDuration = impl_->testRunnerFinished - impl_->testRunnerStarted;
		std::cout << "TestManager finished at " << to_simple_string(impl_->testRunnerStarted) << std::endl;
		std::cout << "Total Duration: " << to_simple_string(impl_->testRunnerDuration) << std::endl;

		impl_->printStatistics();
	}


	// global test API functions

	void internal::reportTestResult(TestPtr test, const char* expression, const char* file, unsigned line, bool fatal, bool success)
	{
		TestResultPtr result = TestResultPtr(new TestResult(test, expression, file, line, fatal, success));
		TestManagerFactory::getTestManager()->reportTestResult(result);
	}
}
