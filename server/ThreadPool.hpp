

#include <pthread.h>
#include <queue>
#include <functional>
#include <iostream>
#include <stdexcept>

using namespace std;

struct Task
{
    using funcv = std::function<void()>;

    Task() = default;
    Task(funcv f) : function(std::move(f)) {}

    funcv function;

    void execute()
    {
        if (function)
        {
            try
            {
                function();
            }
            catch (const std::exception &e)
            {
                cerr << "任务异常: " << e.what() << endl;
            }
        }
    }
};

class ThreadPool
{
public:
    ThreadPool(size_t number) : stop(false)
    {
        pthread_mutex_init(&mutex, nullptr);
        pthread_cond_init(&cond, nullptr);

        workers.resize(number);
        for (size_t i = 0; i < number; i++)
        {
            if (pthread_create(&workers[i], nullptr, work, this) != 0)
            {
                throw std::runtime_error("创建线程失败");
            }
        }
    }

    ~ThreadPool()
    {
        {
            pthread_mutex_lock(&mutex);
            stop = true;
            pthread_mutex_unlock(&mutex);
        }

        pthread_cond_broadcast(&cond);

        for (pthread_t &work : workers)
        {
            pthread_join(work, nullptr);
        }

        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    void addTask(const Task &task)
    {
        pthread_mutex_lock(&mutex);
        if (stop)
        {
            pthread_mutex_unlock(&mutex);
            throw std::runtime_error("向已停止的线程池添加任务");
        }
        taskqueue.push(task);
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&cond);
    }

private:
    vector<pthread_t> workers;
    queue<Task> taskqueue;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool stop;

    static void *work(void *arg)
    {
        ThreadPool *pool = static_cast<ThreadPool *>(arg);
        while (true)
        {
            pthread_mutex_lock(&pool->mutex);

            // 等待任务或停止信号
            while (!pool->stop && pool->taskqueue.empty())
            {
                pthread_cond_wait(&pool->cond, &pool->mutex);
            }

            if (pool->stop && pool->taskqueue.empty())
            {
                pthread_mutex_unlock(&pool->mutex);
                break;
            }

            Task task = std::move(pool->taskqueue.front());
            pool->taskqueue.pop();
            pthread_mutex_unlock(&pool->mutex);

            task.execute();
        }
        return nullptr;
    }
};
