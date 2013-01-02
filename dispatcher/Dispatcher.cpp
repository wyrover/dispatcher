/*
	Dispatcher
	Copyright (c) 2012 Russell Bewley

	Dispatcher is free software released under the MIT License
	(http://www.opensource.org/licenses/mit-license.php)
*/
/*!
	\file Dispatcher.cpp 
	\brief Implementation of the Dispatcher class. 
*/

#include "Dispatcher.hpp"
#include <queue>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/thread.hpp>

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


/*!
	The real Dispatcher class, with all the dependencies.
	
	\note All methods are thread-safe unless otherwise noted.
*/
class Dispatcher::Impl : boost::noncopyable {
public:
	typedef Dispatcher::TaskPtr TaskPtr;
	typedef boost::shared_ptr<boost::thread> ThreadPtr;	

	Impl(): 
		thread_(),
		threadMutex_(),
		stopRequested_(false),
		tasks_(),
		queueMutex_(),
		dataReadyCondition_()
	{
	}

	Impl(bool startImmediately): 
		thread_(),
		threadMutex_(),
		stopRequested_(false),
		tasks_(),
		queueMutex_(),
		dataReadyCondition_()
	{
		if(startImmediately)
		{
			start();
		}
	}

	~Impl()
	{
		stop();
	}

	void start()
	{
		boost::lock_guard<boost::recursive_mutex> threadLock(threadMutex_);
		if(!isThreadRunning(thread_))
		{
			stopRequested_ = false;
			createWorkerThread();
		}
	}

	void stop()
	{
		boost::lock_guard<boost::recursive_mutex> threadLock(threadMutex_);
		if(isThreadRunning(thread_))
		{
			stopWorkerThread();		
			destroyWorkerThread();
		}
	}

	void dispatch(TaskPtr task)
	{
		if(isTaskValid(task))
		{
			boost::lock_guard<boost::mutex> queueLock(queueMutex_);
			tasks_.push(task);
			dataReadyCondition_.notify_all();
		}
	}

	void clear()
	{
		boost::lock_guard<boost::mutex> queueLock(queueMutex_);
		while(!tasks_.empty())
		{
			tasks_.pop();
		}
	}

	bool isRunning()
	{		
		boost::lock_guard<boost::recursive_mutex> threadLock(threadMutex_);
		return isThreadRunning(thread_);
	}

	size_t size()
	{
		size_t n = 0;
		boost::lock_guard<boost::mutex> queueLock(queueMutex_);
		n = tasks_.size();
		return(n);
	}

private:
	void run()
	{
		try
		{
			TaskPtr task;
			while(!stopRequested_)
			{
				boost::this_thread::interruption_point();

				task = getNextTask();

				// execute the task, or wait for a task to become available
				if(isTaskValid(task))
				{
					bool recur = false;

					// run the task
					// note: recurring tasks will tell us when to run them
					if(!task->isRecurring() || task->shouldRecur())
					{
						recur = task->run();

						// ignore the return value of run for recurring tasks
						if(task->isRecurring())
						{
							recur = true;
						}
					}

					// are we done with the task, or
					// do we need to keep it around?
					if(recur)
					{
						// the task needs to be run again, so add
						// it back to the queue.
						dispatch(task);
					}
					else
					{
						// the task does not need to be run again,
						// so we can get rid of it.
						task.reset();
					}
				}
				else if(!task && !stopRequested_)
				{
					boost::unique_lock<boost::mutex> dataReadyLock(dataReadyMutex_);
					dataReadyCondition_.wait(dataReadyLock);
				}
			}
		}
		catch(boost::thread_interrupted&)
		{
			stopRequested_ = true;
		}
	}

	static bool isTaskValid(TaskPtr task)
	{
		return(task && task->isValid());
	}

	TaskPtr getNextTask()
	{
		TaskPtr task;
		boost::lock_guard<boost::mutex> queueLock(queueMutex_);
		if(!tasks_.empty())
		{
			task = tasks_.front();
			tasks_.pop();
		}
		return(task);
	}

	// not thread-safe
	void createWorkerThread()
	{
		thread_ = ThreadPtr(new boost::thread(&Impl::run, this));
	}

	// not thread-safe
	void destroyWorkerThread()
	{
		thread_.reset();
	}

	// not thread-safe
	void stopWorkerThread()
	{
		stopRequested_ = true;
		do 
		{
			dataReadyCondition_.notify_all();
		}
		while(false == thread_->timed_join(boost::posix_time::milliseconds(10)));
	}

	// not thread-safe
	static bool isThreadRunning(ThreadPtr thread)
	{		
		bool running = false;
		try
		{
			if(thread && (thread->get_id() != boost::this_thread::get_id()))
			{
				running = !(thread->timed_join(boost::posix_time::milliseconds(0)));
			}
		}
		catch(boost::thread_interrupted&)
		{
			// thrown if thread exits as join is called.
			// this can safely be ignored.
		}
		return(running);
	}

private:	
	ThreadPtr thread_;
	mutable boost::recursive_mutex threadMutex_;
	volatile bool stopRequested_;
	std::queue<TaskPtr> tasks_;
	mutable boost::mutex queueMutex_;
	mutable boost::mutex dataReadyMutex_;
	mutable boost::condition_variable dataReadyCondition_;	
};


/*
 * Dispatcher class implementation
 */

Dispatcher::Dispatcher():
	impl_()
{
	impl_ = boost::shared_ptr<Impl>(new Impl());
}

Dispatcher::Dispatcher(bool startImmediately):
	impl_()
{
	impl_ = boost::shared_ptr<Impl>(new Impl(startImmediately));
}

Dispatcher::~Dispatcher()
{
}

void Dispatcher::start()
{
	impl_->start();
}

void Dispatcher::stop()
{
	impl_->stop();
}

void Dispatcher::dispatch(TaskPtr task)
{
	impl_->dispatch(task);
}

void Dispatcher::clear()
{
	impl_->clear();
}

bool Dispatcher::isRunning()
{
	return impl_->isRunning();
}

size_t Dispatcher::size()
{
	return impl_->size();
}
