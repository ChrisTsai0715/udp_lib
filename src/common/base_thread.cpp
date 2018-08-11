#include "base_thread.h"
#include <iostream>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <signal.h>
#include <sys/select.h>   
#include <unistd.h>

base_thread::base_thread()
: _hThread(0)
, _bRunning(false)
, _bWaitStop(false)
, _bExited(false)
, run_once(false)
{
}

base_thread::~base_thread()
{
}


bool base_thread::RunOnce() {
  return _run(true);
}

bool base_thread::run() {
  return _run(false);
}

bool base_thread::_run(bool once)
{
  if(_bRunning)
    return true;

  run_once = once;
  _bRunning = true;
  _bWaitStop = false;
  _bExited = false;

  if (0 != pthread_create(&_hThread, NULL, &thread_func, this)) {
    _bRunning = false;
    _hThread = 0;
    return false;
  }
  
  return _bRunning;
}
 

bool base_thread::wait_for_timeout(unsigned long timeout)
{
    if(!_bRunning)
	  return true;
 
    struct timeval t_timeval;
    t_timeval.tv_sec = timeout / 1000;
    t_timeval.tv_usec = 0;
    select(0, NULL, NULL, NULL, &t_timeval );

    return true;
}


bool base_thread::wait_for_stop(unsigned long timeout)
{
  if(!_bRunning) return true;

  _bWaitStop = true;

  assert(run_once == false);
  if (!run_once) pthread_join(_hThread, NULL);

  return true;
}

bool base_thread::wait_for_thread_end(unsigned long timeout)
{
  if(!_bRunning) return true;

  assert(run_once == false);
  if (!run_once) pthread_join(_hThread, NULL);
  
  return true;
}

bool base_thread::stop()
{
    if(!_bRunning) return true;

    wait_for_stop();

    _hThread = 0;
    _bRunning = false;
    return true;
}

bool base_thread::is_running() const
{
   return _bRunning && !_bExited;
}

void* base_thread::thread_func( void* pArguments )
{
  base_thread*pThis = (base_thread*)pArguments;

  if(pThis->run_once) {
    pthread_detach(pthread_self());
  }

  unsigned exetcode = 0;
  pThis->on_begin();

  while(!pThis->_bWaitStop)
  {
    if(!pThis->thread_loop() || pThis->run_once)
    {
      exetcode = 0;
      break;
    }
  }

  pThis->_bExited = true;
  pThis->on_exit();

  return NULL;
}
