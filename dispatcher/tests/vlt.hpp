/*
	Very Lite Test (VLT)
	Copyright (c) 2012 Russell Bewley
	
	http://github.com/rbewley4/vlt

	VLT is free software released under the MIT License
	(http://www.opensource.org/licenses/mit-license.php)
*/
/*!
	\file vlt.hpp 
	\brief Main header file for the VLT testing framework. 
*/

#ifndef VLT_INCLUDED_HPP
#define VLT_INCLUDED_HPP

#include <algorithm> //for_each
#include <list>
#include <queue>
#include <string>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp> //noncopyable

//! The main namespace for VLT
namespace test {

	// forward declarations of smart pointer types
	class Test;
	class TestResult;
	class TestEventListener;
	class TestManager;
	typedef boost::shared_ptr<Test>              TestPtr;	
	typedef boost::shared_ptr<TestResult>        TestResultPtr;	
	typedef boost::shared_ptr<TestEventListener> TestEventListenerPtr;
	typedef boost::shared_ptr<TestManager>       TestManagerPtr;

	//! Interface for a test case
	class Test
	{
	public:
		virtual ~Test() {}
		virtual void execute() = 0;
		virtual std::string group() const = 0;
		virtual std::string name() const = 0;
	};

	//! A class that encapsulates the result of a test assertion.
	class TestResult {
	public:
		TestResult(
			TestPtr test,
			const std::string& expression, 
			const std::string& file, 
			unsigned line, 
			bool fatal, 
			bool success);
		TestResult(const TestResult& rhs);
		~TestResult();
		TestResult& operator=(const TestResult& rhs);

		TestPtr getTest() const;
		const std::string& getExpression() const;
		const std::string& getFile() const;
		unsigned getLine() const;
		bool isFatal() const;
		bool isSuccess() const;
		bool isFailure() const;

	private:
		TestPtr test_;
		std::string expression_;
		std::string file_;
		unsigned line_;
		bool fatal_;
		bool success_;
	};

	//! An interface to handle test events.
	class TestEventListener {
	public:
		virtual ~TestEventListener(){}
		
		virtual void onTestAdded(TestPtr test) = 0;
		virtual void onTestingStarted() = 0;
		virtual void onTestAboutToRun(TestPtr test) = 0;
		virtual void onTestResult(TestResultPtr result) = 0;
		virtual void onTestFinished(TestPtr test) = 0;
		virtual void onTestingStopped() = 0;
	};

	/*!
		A generic event manager that can maintain a
		list of clients, and notify them when events
		are raised. Works with both smart pointers
		and regular pointers.
	*/
	template<typename EventListenerPtrType>
	class EventManager : public boost::noncopyable {
	public:
		typedef boost::function1<void, EventListenerPtrType> Event;

		EventManager() : eventListeners_() 
		{}

		~EventManager() 
		{ 
			removeAll();
		}

		const std::list<EventListenerPtrType>& getEventListeners() const
		{
			return(eventListeners_);
		}

		void add(EventListenerPtrType eventListener)
		{
			eventListeners_.push_back(eventListener);
		}

		void remove(EventListenerPtrType eventListener)
		{
			eventListeners_.remove(eventListener);
		}

		void removeAll()
		{
			eventListeners_.clear();
		}

		//! invoke a member function on all listeners
		void raise(Event event) const
		{
				std::for_each(eventListeners_.begin(), eventListeners_.end(),
				event);
		}

	private:
		std::list<EventListenerPtrType> eventListeners_;
	};

	//! convenience method to raise an event with no args
	template<typename EventListenerPtrType, typename EventListenerMemberFunctionType>
	void raise(const EventManager<EventListenerPtrType>& eventManager, 
		EventListenerMemberFunctionType callback)
	{
		EventManager<EventListenerPtrType>::Event event = boost::bind(callback, _1);
		eventManager.raise(event);
	}

	//! convenience method to raise an event with 1 arg
	template<typename EventListenerPtrType, typename EventListenerMemberFunctionType, typename ArgType>
	void raise(const EventManager<EventListenerPtrType>& eventManager, 
		EventListenerMemberFunctionType callback, ArgType arg)
	{
		EventManager<EventListenerPtrType>::Event event = boost::bind(callback, _1, arg);
		eventManager.raise(event);
	}

	/*!
		The brains behind the operation. In charge
		of running the tests, and notifying the
		TestEventListener of events.
	*/
	class TestManager : public boost::noncopyable {
	public:
		TestManager();
		~TestManager();

		void addTest(TestPtr test);
		void runTests();
		void stopTests();		
		void reportTestResult(TestResultPtr result);

		TestPtr getCurrentTest() const;

		EventManager<TestEventListenerPtr>& getEventManager();

	private:
		void runOne(TestPtr test);

		std::queue<TestPtr> tests_;
		bool stopTestsIndicator_;
		TestPtr currentTest_;
		EventManager<TestEventListenerPtr> eventManager_;
	};

	//! A factory to manage the TestManager singleton
	class TestManagerFactory : public boost::noncopyable {
	public:
		static TestManagerPtr getTestManager();
	};

	/*!
		A factory to instantiate classes that implement
		the Test interface, and register the instance
		with a TestManager.
	*/
	class TestFactory : public boost::noncopyable
	{
	public:
		template<typename TestImpl>
		static TestPtr createAndRegisterTest(TestManagerPtr testManager)
		{
			TestPtr test(new TestImpl());
			testManager->addTest(test);
			return(test);
		}
	};
	
//! VLT implementation details
namespace internal {

	/*!
		Partial implementation of the Test interface
		that is used as a base (abstract) class by 
		the macros to create test classes.
	*/
	class BaseTest : public Test
	{
	public:
		BaseTest(const std::string& group, const std::string& name);
		BaseTest(const BaseTest& rhs);
		virtual ~BaseTest();
		virtual void execute() = 0;
		std::string group() const;
		std::string name() const;
	private:
		std::string group_;
		std::string name_;
	};

	/*!
		Reports test results to stdout and stderr.
	*/
	class DefaultEventListener : public TestEventListener, public boost::noncopyable {
	public:
		DefaultEventListener();
		virtual ~DefaultEventListener();

		virtual void onTestAdded(TestPtr test);
		virtual void onTestingStarted();
		virtual void onTestAboutToRun(TestPtr test);
		virtual void onTestResult(TestResultPtr result);
		virtual void onTestFinished(TestPtr test);
		virtual void onTestingStopped();

	private:
		class Impl;
		boost::shared_ptr<Impl> impl_;
	};
	
	/*!
		A utility method to create TestResults and
		report them to the TestManager singleton.

		This is called by the assertion macros to
		facilitate reporting test results.
	*/
	void reportTestResult(TestPtr test, const char* expression, const char* file, unsigned line, bool fatal, bool success);

} //namespace internal
} //namespace test


// test creation macros

//! Adds a test to the TestManager singleton
#define ADD_TEST(t) test::TestManagerFactory::getTestManager()->addTest(t)

//! Returns the name of a test class built from a group and test name.
#define TEST_CLASS_NAME(group, name) Test_##group##_##name

//! Defines a test class, instantiates it, and opens up the definition of the execute() method
#define TEST(group, name) \
	class TEST_CLASS_NAME(group, name) : public test::internal::BaseTest, public boost::noncopyable \
	{ \
	public: \
		TEST_CLASS_NAME(group, name)() : test::internal::BaseTest(#group, #name) {} \
		~TEST_CLASS_NAME(group, name)() {} \
		virtual void execute(); \
	}; \
	test::TestPtr TEST_CLASS_NAME(group, name)_ = \
		test::TestFactory::createAndRegisterTest<TEST_CLASS_NAME(group, name)>( \
			test::TestManagerFactory::getTestManager()); \
	void TEST_CLASS_NAME(group, name)::execute()

//! Makes the TestManager singleton run all tests
#define RUN_TESTS() test::TestManagerFactory::getTestManager()->runTests()


// test assertion macros

#define TEST_ASSERT__(expression__, fatal__) \
	test::internal::reportTestResult(test::TestManagerFactory::getTestManager()->getCurrentTest(), \
   #expression__, __FILE__, __LINE__, fatal__, !!(expression__))

#define TEST_ASSERT(expression__) TEST_ASSERT__(expression__, false)

#define TEST_ASSERT_FATAL(expression__) TEST_ASSERT__(expression__, true)

#define TEST_EQUALS(a__,b__) TEST_ASSERT(a__ == b__)

#define TEST_NOT_EQUALS(a__,b__) TEST_ASSERT(a__ != b__)

#define TEST_LESS_THAN(a__,b__) TEST_ASSERT(a__ < b__)

#define TEST_GREATER_THAN(a__,b__) TEST_ASSERT(a__ > b__)

#define TEST_LESS_THAN_OR_EQUAL_TO(a__,b__) TEST_ASSERT(a__ <= b__)

#define TEST_GREATER_THAN_OR_EQUAL_TO(a__,b__) TEST_ASSERT(a__ >= b__)

#define TEST_IS_NULL(a__) TEST_EQUALS(a__, 0)

#define TEST_NOT_NULL(a__) TEST_NOT_EQUALS(a__, 0)

#define TEST_FAIL() TEST_ASSERT(false)

#define TEST_PASS() TEST_ASSERT(true)

//! includes endpoints
#define TEST_IN_CLOSED_INTERVAL(x, min, max) TEST_ASSERT((min <= x) && (x <= max))

//! does not include endpoints
#define TEST_IN_OPEN_INTERVAL(x, min, max) TEST_ASSERT((min < x) && (x < max))

#define TEST_IN_RANGE(x, min, max) TEST_IN_CLOSED_INTERVAL(x, min, max)

#endif //VLT_INCLUDED_HPP
