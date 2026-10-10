#pragma once

#include <mutex>
#include <atomic>
#include <condition_variable>
#include <exception>
#include <functional>
#include <queue>
#include <thread>
#include <vector>

namespace parallel_detail {
inline std::size_t workerCount()
{
  const auto detected = std::thread::hardware_concurrency();
  return detected == 0 ? 1 : static_cast<std::size_t>(detected);
}
}

class BarrierWithRunLoop
{
public:
  BarrierWithRunLoop() = default;
  ~BarrierWithRunLoop() = default;

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
    std::exception_ptr jobException;
    const auto runJob = [&](std::function<void()> job) {
      lk.unlock();
      try {
        job();
      }
      catch(...) {
        if(!jobException)
          jobException = std::current_exception();
      }
      lk.lock();
    };
    cv.wait(lk, [&]{
      while(!jobs.empty()){
        auto job = std::move(jobs.front());
        jobs.pop();
        runJob(std::move(job));
      }
      return count == 0;
    });
    // dependent threads finished
    // completing last posted jobs
    while(!jobs.empty()){
      auto job = std::move(jobs.front());
      jobs.pop();
      runJob(std::move(job));
    }
    if(jobException)
      std::rethrow_exception(jobException);
  }

  std::mutex cv_m; // mutex is public - sometimes it can be usefull in threads
private:
  std::condition_variable cv;
  std::atomic_int count = 0;
  std::queue<std::function<void()>> jobs;
};

template<typename T>
void parallelWithRunLoop(T worker)
{
  BarrierWithRunLoop bwc;

  const std::size_t threads = parallel_detail::workerCount();
  std::vector<std::thread> workers;
  workers.reserve(threads);
  std::mutex exceptionMutex;
  std::exception_ptr workerException;
  for(size_t i=0; i<threads; i++)
  {
    bwc.lock();
    workers.emplace_back([&, i]{
      try {
        worker(threads, i, bwc);
      }
      catch(...) {
        std::scoped_lock lock(exceptionMutex);
        if(!workerException)
          workerException = std::current_exception();
      }
      bwc.unlock();
    });
  }
  std::exception_ptr runLoopException;
  try {
    bwc.wait();
  }
  catch(...) {
    runLoopException = std::current_exception();
  }
  for(auto& thread : workers)
    thread.join();
  if(runLoopException)
    std::rethrow_exception(runLoopException);
  if(workerException)
    std::rethrow_exception(workerException);
};

//##################################################################################################
template<typename T>
void parallel(T worker)
{
    const std::size_t threads = parallel_detail::workerCount();
    std::vector<std::thread> workers;
    workers.reserve(threads);
    std::mutex exceptionMutex;
    std::exception_ptr workerException;
    for(size_t i=0; i<threads; i++)
        workers.emplace_back([&, i]{
          try {
            worker(i);
          }
          catch(...) {
            std::scoped_lock lock(exceptionMutex);
            if(!workerException)
              workerException = std::current_exception();
          }
        });
    for(auto& thread : workers)
        thread.join();
    if(workerException)
      std::rethrow_exception(workerException);
}
