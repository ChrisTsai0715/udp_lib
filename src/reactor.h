#ifndef _REACTOR_CAIRUI_H_
#define _REACTOR_CAIRUI_H_

#include "base_thread.h"
#include <list>
#include <memory>
#include <sys/time.h>
#include <atomic>
#include "cslock.h"
#include "base_net.h"

namespace net_lib {

	typedef enum
	{
		TASK_TYPE_TIMER,
		TASK_TYPE_RECV,
		TASK_TYPE_SEND,
	}task_type_def;

	class base_task
	{
	public:
		base_task(task_type_def type)
			:	_type(type)
		{}
		virtual ~base_task(void) {}

		virtual bool done(void) = 0;
		task_type_def get_type(void) const {return _type;}

	protected:
		task_type_def _type;
	};

    class net_task : public base_task
	{
	public:
        net_task (std::shared_ptr<base_net> net);
        virtual ~net_task(void);

        virtual bool done(void);

        virtual int get_sockfd(void) const {return  _net->get_sockfd();}

    private:
        std::shared_ptr<base_net> _net;
	};

	class timer_task : public base_task
	{
	public:
		timer_task(void)
			:	base_task(TASK_TYPE_TIMER)
		{}
        virtual ~timer_task(void) {}
        virtual uint64_t get_execute_time(void) const {return _execute_time;}
        virtual void set_execute_time(uint64_t time) {_execute_time = time;}

	protected:
        uint64_t _execute_time;
	};

    template<typename FUNC_TYPE>
	class timer_task_void : public timer_task
	{
	public:
        timer_task_void(FUNC_TYPE func)
			:	_done_func(func)
		{}
        virtual ~timer_task_void(void) {}
		virtual bool done(void)
		{
			return _done_func();
		}

	private:
        FUNC_TYPE _done_func;
	};

    template<typename FUNC_TYPE>
    static std::shared_ptr<base_task> create_task(FUNC_TYPE func)
	{
        return std::make_shared<timer_task_void<FUNC_TYPE>>(func);
	}

	class reactor
	{
	public:
		reactor(void);
		virtual ~reactor(void);

		bool run();
		bool stop();

		bool add_recv_task(std::shared_ptr<base_task> task);
        bool add_timer_task(std::shared_ptr<base_task> task, uint32_t time_ms);
        bool remove_task(std::shared_ptr<base_task> task);

    private:
        bool _add_task(std::shared_ptr<base_task> task);
        bool _remove_task(std::shared_ptr<base_task> task);

	private:
		class work_thread : private base_thread
		{
		public:
			work_thread(reactor *outer)
				:	_outer(outer),
					_task_rerun(false)
			{}
			virtual ~work_thread(void) {}

            bool set_rerun(bool rerun) {_task_rerun = rerun;}
			virtual bool run(void) {return base_thread::run();}
			virtual bool stop(void) {return base_thread::stop();}
			virtual bool thread_loop(void);

		private:
			reactor *_outer;
            std::atomic_bool _task_rerun;
        } _work_thread;

	private:
		std::list<std::shared_ptr<base_task>> _task_list;
		CMutexLock _mutex;

		friend class work_thread;
	};

}


#endif
