#pragma once

#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <queue>

class BarrierWithCounter
{
public:
  BarrierWithCounter() = default;

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
    cv.notify_all();
  }

  void wait()
  {
    std::unique_lock<std::mutex> lk(cv_m);
    cv.wait(lk, [&]{
      while(!jobs.empty()){
        auto job = jobs.front();
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

template<typename T>
void parallelWithBarrier(T worker)
{
  BarrierWithCounter bwc;

  size_t threads = std::thread::hardware_concurrency();
  std::vector<std::thread> workers;
  workers.reserve(threads);
  for(size_t i=0; i<threads; i++)
  {
    bwc.lock();
    workers.emplace_back([&]{ worker(i, bwc); bwc.unlock(); });
  }
  bwc.wait();
  for(auto& thread : workers)
    thread.join();
};
