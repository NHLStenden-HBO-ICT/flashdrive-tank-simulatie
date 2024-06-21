#pragma once

namespace Tmpl8
{

class ThreadPool; 

class Worker;

class Worker
{
  public:
   
    Worker(ThreadPool& s) : pool(s) {}

    inline void operator()();

  private:
    ThreadPool& pool;
};

class ThreadPool
{
  public:
    ThreadPool(size_t numThreads) : stop(false)
    {
        for (size_t i = 0; i < numThreads; ++i)
            workers.push_back(std::thread(Worker(*this)));
    }

    ~ThreadPool()
    {
        stop = true; 
        condition.notify_all();

        for (auto& thread : workers)
            thread.join();
    }

    template <class T>
    auto enqueue(T task) -> std::future<decltype(task())>
    {
        auto wrapper = std::make_shared<std::packaged_task<decltype(task())()>>(std::move(task));

        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            tasks.push_back([=] {
                (*wrapper)();
            });
        }

        condition.notify_one();

        return wrapper->get_future();
    }

  private:
    friend class Worker; 

    std::vector<std::thread> workers;
    std::deque<std::function<void()>> tasks;

    std::condition_variable condition; 

    std::mutex queue_mutex; 
    bool stop = false;
};

inline void Worker::operator()()
{
    std::function<void()> task;
    while (true)
    {
        
        {
            std::unique_lock<std::mutex> locker(pool.queue_mutex);

            pool.condition.wait(locker, [=] { return pool.stop || !pool.tasks.empty(); });

            if (pool.stop) break;

            task = pool.tasks.front();
            pool.tasks.pop_front();
        }

        task();
    }
}

} 