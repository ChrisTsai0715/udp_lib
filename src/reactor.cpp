#include "reactor.h"
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

using namespace net_lib;

reactor::reactor(void)
    :	_work_thread(this)
{

}

reactor::~reactor(void)
{
}

bool reactor::run(void)
{
	return _work_thread.run();
}

bool reactor::stop(void)
{

}

bool reactor::add_recv_task(std::shared_ptr<base_task> task)
{
    return _add_task(task);
}

bool reactor::add_timer_task(std::shared_ptr<base_task> task, uint32_t time_ms)
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);

    std::shared_ptr<timer_task> _task = std::dynamic_pointer_cast<timer_task>(task);
    _task->set_execute_time(tv.tv_usec + time_ms * 1000);

    return _add_task(task);
}

bool reactor::remove_task(std::shared_ptr<base_task> task)
{
    _work_thread.set_rerun(true);
    return _remove_task(task);
}

bool reactor::_add_task(std::shared_ptr<base_task> task)
{
    CAutoLockEx<CMutexLock> cslock(_mutex);

    _task_list.push_back(task);

    return true;
}

bool reactor::_remove_task(std::shared_ptr<base_task> task)
{
    CAutoLockEx<CMutexLock> cslock(_mutex);
    _task_list.remove(task);

    return true;
}

bool reactor::work_thread::thread_loop(void)
{
	_task_rerun = false;
	std::list<std::shared_ptr<base_task>> tmp_list;
	{
        CAutoLockEx<CMutexLock> cslock(_outer->_mutex);
        tmp_list = _outer->_task_list;
	}

    fd_set recv_set, send_set;
    int max_fd = 0;
    FD_ZERO(&recv_set);
    FD_ZERO(&send_set);

    auto it = tmp_list.begin();
    for (; it != tmp_list.end(); it ++)
    {
        if (_task_rerun) return true;

        switch ((*it)->get_type())
        {
        case TASK_TYPE_TIMER:
        {
            std::shared_ptr<timer_task> task = std::dynamic_pointer_cast<timer_task>((*it));
            if (task == nullptr) break;
            struct timeval tv;
            gettimeofday(&tv, nullptr);
            if (task->get_execute_time() <= (uint64_t)tv.tv_usec)
            {
                task->done();
                _outer->_remove_task(task);
            }
        }
        break;

        case TASK_TYPE_RECV:
        {
            std::shared_ptr<net_task> task = std::dynamic_pointer_cast<net_task>((*it));
            if (max_fd < task->get_sockfd()) max_fd = task->get_sockfd();
            FD_SET(task->get_sockfd(), &recv_set);
        }
        break;

        default:
            break;
        }
    }

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 300 * 1000;

    int ret = ::select(max_fd + 1, &recv_set, &send_set, nullptr, &timeout);
    if (ret > 0)
    {
        if (_task_rerun) return true;

        it = tmp_list.begin();
        while(it != tmp_list.end())
        {
            if ((*it)->get_type() == TASK_TYPE_RECV)
            {
                std::shared_ptr<net_task> task = std::dynamic_pointer_cast<net_task>((*it));
                if (FD_ISSET(task->get_sockfd(), &recv_set))
                {
                    (*it)->done();
                    _outer->_remove_task(task);
                }
            }

            it ++;
        }
    }

	return true;
}

net_task::net_task(std::shared_ptr<base_net> net)
    :	base_task(TASK_TYPE_RECV),
        _net(net)
{

}

net_task::~net_task()
{

}

bool net_task::done()
{
    return _net->recv();
}
