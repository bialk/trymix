#ifndef BARRIERWITHCOUNTER_H
#define BARRIERWITHCOUNTER_H

#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <queue>

namespace tp_utils {

class BarrierWithCounter
{
public:
  BarrierWithCounter()
  {
  }

  void lock()
  {
    ++count;
  }

  void unlock()
  {
    --count;
    cv.notify_all();
  }

  void run(std::function<void()> const& job)
  {
    std::scoped_lock<std::mutex> lk(cv_m);
    jobs.push(job);
  }

  void wait()
  {
    std::unique_lock<std::mutex> lk(cv_m);
    cv.wait(lk, [&]{
      while(!jobs.empty()){
        auto job = jobs.back();
        jobs.pop();
        lk.unlock();
        job();
        lk.lock();
      }
      return count == 0;
    });
    // dependent threads finished
    // completing last posted jobs
    while(!jobs.empty()){
      jobs.back()();
      jobs.pop();
    }
  }

  std::mutex cv_m; // mutex is public - sometimes it can be usefull in threads
private:
  std::condition_variable cv;
  std::atomic_int count = 0;
  std::queue<std::function<void()>> jobs;
};

}

#endif // BARRIERWITHCOUNTER_H
