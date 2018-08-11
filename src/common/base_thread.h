#ifndef _BASE_THREAD_CAIRUI_
#define _BASE_THREAD_CAIRUI_

#include <pthread.h>
#include <time.h>
#include <string>

class base_thread
{
public:
  base_thread();
  virtual ~base_thread();

public:
  // normal thread start 
  virtual bool run();
  // dettached thread start with no calling "Stop()"
  virtual bool RunOnce();
  virtual bool stop();
  bool is_running() const;
  bool wait_for_thread_end(unsigned long timeout = -1);

public:

  pthread_t _hThread;

private:
  virtual bool _run(bool once=false);

protected:
  virtual bool thread_loop() = 0;
  virtual bool wait_for_stop(unsigned long timeout = -1);
  bool wait_for_timeout(unsigned long timeout);
  virtual void on_exit(){}
  virtual void on_begin(){}

  volatile bool _bRunning;
  volatile bool _bWaitStop;
  volatile bool _bExited;
  volatile bool run_once;

  static void* thread_func( void* pArguments );
  
};

#endif

