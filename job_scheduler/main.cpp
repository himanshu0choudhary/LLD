#include "bits/stdc++.h"
using namespace std;

class Job
{
public:
    virtual void execute() = 0;
    virtual ~Job() = default;
};

class ThreadPool
{
private:
    vector<thread> workers;
    queue<shared_ptr<Job>> tasks;
    mutex mtx;
    condition_variable cv;
    bool stopping = false;

public:
    ThreadPool(int n)
    {
        for (int i = 0; i < n; i++)
        {
            workers.emplace_back([this]()
                                 {
                while (true) {
                    shared_ptr<Job> job;
                    {
                        unique_lock<mutex> lock(mtx);
                        cv.wait(lock, [this]() { return stopping || !tasks.empty(); });
                        if (stopping && tasks.empty()) return;
                        job = std::move(tasks.front());
                        tasks.pop();
                    }
                    if (job) job->execute();
                } });
        }
    }

    void push(shared_ptr<Job> job)
    {
        {
            lock_guard<mutex> lock(mtx);
            if (stopping)
                return;
            tasks.push(std::move(job));
        }
        cv.notify_one();
    }

    ~ThreadPool()
    {
        {
            lock_guard<mutex> lock(mtx);
            stopping = true;
        }
        cv.notify_all();
        for (auto &t : workers)
            if (t.joinable())
                t.join();
    }
};

struct ScheduledJob
{
    shared_ptr<Job> job;
    chrono::steady_clock::time_point nextExecution;
    int priority;
    bool isRecurring = false;
    int intervalS = 0;
};

class JobManager
{
private:
    // Comparators
    struct DelayCmp
    {
        bool operator()(const shared_ptr<ScheduledJob> &a, const shared_ptr<ScheduledJob> &b)
        {
            return a->nextExecution > b->nextExecution; // Earliest first
        }
    };

    struct ReadyCmp
    {
        bool operator()(const shared_ptr<ScheduledJob> &a, const shared_ptr<ScheduledJob> &b)
        {
            return a->priority < b->priority; // Highest priority first
        }
    };

    priority_queue<shared_ptr<ScheduledJob>, vector<shared_ptr<ScheduledJob>>, DelayCmp> delayPq;
    priority_queue<shared_ptr<ScheduledJob>, vector<shared_ptr<ScheduledJob>>, ReadyCmp> readyPq;

    shared_ptr<ThreadPool> pool;
    thread schedulerThread;
    mutex mtx;
    condition_variable cv;
    bool stop = false;

public:
    JobManager(shared_ptr<ThreadPool> pool) : pool(pool)
    {
        schedulerThread = thread([this]()
                                 {
            while (true) {
                unique_lock<mutex> lock(mtx);
                
                if (stop && delayPq.empty()) break;

                if (delayPq.empty()) {
                    cv.wait(lock, [this] { return stop || !delayPq.empty(); });
                    if (stop && delayPq.empty()) break;
                }
                else{
                    auto nextTime = delayPq.top()->nextExecution;
                    cv.wait_until(lock, nextTime);
                }

                auto now = chrono::steady_clock::now();
                bool addedToReady = false;

                while (!delayPq.empty() && delayPq.top()->nextExecution <= now) {
                    auto top = delayPq.top();
                    delayPq.pop();
                    
                    readyPq.push(top);
                    addedToReady = true;

                    if (top->isRecurring) {
                        // Reschedule recurring job
                        top->nextExecution = now + chrono::seconds(top->intervalS);
                        delayPq.push(top);
                    }
                }

                if (addedToReady) {
                    // Push everything from readyPq to the ThreadPool
                    while (!readyPq.empty()) {
                        this->pool->push(readyPq.top()->job);
                        readyPq.pop();
                    }
                }
            } });
    }

    void submit(shared_ptr<ScheduledJob> jobSchedule)
    {
        lock_guard<mutex> lock(mtx);
        delayPq.push(jobSchedule);
        cv.notify_one(); // Wake up scheduler to re-evaluate wait time
    }

    ~JobManager()
    {
        {
            lock_guard<mutex> lock(mtx);
            stop = true;
        }
        cv.notify_all();
        if (schedulerThread.joinable())
            schedulerThread.join();
    }
};

class SimpleJob : public Job
{
private:
    int id;
    int durationS;

public:
    SimpleJob(int id, int durationS) : id(id), durationS(durationS) {}

    void execute() override
    {
        cout << "Starting: " << id << endl;
        this_thread::sleep_for(chrono::seconds(durationS));
        cout << "Ended: " << id << endl;
    }
};

int main()
{
    shared_ptr<ThreadPool> pool = make_shared<ThreadPool>(2);

    JobManager jobManager(pool);

    auto now = chrono::steady_clock::now();

    shared_ptr<ScheduledJob> s1 = make_shared<ScheduledJob>();
    s1->intervalS = 1;
    s1->nextExecution = now + chrono::seconds(2);
    s1->job = make_shared<SimpleJob>(1, 3);
    s1->priority = 1;

    shared_ptr<ScheduledJob> s2 = make_shared<ScheduledJob>();
    s2->intervalS = 5;
    s2->nextExecution = now + chrono::seconds(2);
    s2->job = make_shared<SimpleJob>(2, 3);
    s2->priority = 3;
    s2->isRecurring = true;

    jobManager.submit(s1);
    jobManager.submit(s2);

    this_thread::sleep_for(chrono::seconds(60));
}