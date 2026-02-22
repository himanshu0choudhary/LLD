#include "bits/stdc++.h"
using namespace std;

struct Message
{
    string content;
};

class QueueTopic
{
private:
    string name;

    class InternalMessage
    {
    public:
        size_t id;
        Message message;
        chrono::steady_clock::time_point valid_till;

        InternalMessage(int id, Message msg, chrono::steady_clock::time_point valid_till) : id(id), message(std::move(msg)), valid_till(valid_till) {}
    };

    deque<InternalMessage> messageQueue;
    mutex mtx;

    chrono::milliseconds ttl;

    size_t messageId = 0;

public:
    QueueTopic(string name, chrono::milliseconds ttl) : name(name), ttl(ttl) {}

    void push(Message msg)
    {
        lock_guard<mutex> lock(mtx);

        InternalMessage internalMessage(messageId++, std::move(msg), chrono::steady_clock::now() + ttl);
        messageQueue.push_back(std::move(internalMessage));
    }

    vector<Message> get(size_t &offset, int batchSize)
    {
        lock_guard<mutex> lock(mtx);
        vector<Message> msgs;
        auto now = chrono::steady_clock::now();

        if (messageQueue.empty())
            return {};

        size_t minId = messageQueue.front().id;
        offset = max(offset, minId);

        while ((offset - minId) < messageQueue.size() and batchSize > 0)
        {
            msgs.push_back(messageQueue[offset - minId].message);
            offset++;
            batchSize--;
        }
        return msgs;
    }

    void cleanup()
    {
        lock_guard<mutex> lock(mtx);
        auto now = chrono::steady_clock::now();

        // Efficiently remove only from the front to maintain index integrity
        auto it = messageQueue.begin();
        while (it != messageQueue.end() && it->valid_till < now)
        {
            it++;
        }
        messageQueue.erase(messageQueue.begin(), it);
    }
};

class MessageQueueService
{
private:
    unordered_map<string, shared_ptr<QueueTopic>> topicMap;

    shared_timed_mutex sharedMtx;

    chrono::milliseconds ttl;

    thread cleanupThread;
    atomic<bool> stop{false};

    mutex cvMtx;
    condition_variable cv;

public:
    MessageQueueService(chrono::milliseconds ttl) : ttl(ttl)
    {
        cleanupThread = thread([this]()
                               {
            while(true){
                {
                    unique_lock<mutex> lock(cvMtx);
                    cv.wait_for(lock, chrono::seconds(3), [this](){ return stop.load();});
                }
                
                if(stop)
                    break;

                shared_lock<shared_timed_mutex> lock(sharedMtx);
                for(auto it: topicMap) it.second->cleanup();
            } });
    }

    ~MessageQueueService()
    {
        {
            lock_guard<shared_timed_mutex> lock(sharedMtx);
            stop = true;
        }

        cv.notify_all();

        if (cleanupThread.joinable())
            cleanupThread.join();
    }

    void createTopic(string name)
    {
        lock_guard<shared_timed_mutex> lock(sharedMtx);

        if (stop)
            return;

        auto it = topicMap.find(name);

        if (it == topicMap.end())
        {
            topicMap.insert({name, make_shared<QueueTopic>(name, ttl)});
        }
    }

    void push(string topicName, Message msg)
    {
        shared_lock<shared_timed_mutex> lock(sharedMtx);

        if (stop)
            return;

        auto it = topicMap.find(topicName);

        if (it != topicMap.end())
        {
            it->second->push(std::move(msg));
        }
    }

    vector<Message> read(string topicName, size_t &offset, int batchSize)
    {
        shared_lock<shared_timed_mutex> lock(sharedMtx);

        if (stop)
            return {};

        auto it = topicMap.find(topicName);

        if (it != topicMap.end())
        {
            auto res = (it->second)->get(offset, batchSize);
            return res;
        }
        return {};
    }
};

int main()
{
    MessageQueueService msgQ(chrono::milliseconds(2000));

    thread t1([&]()
              {
        msgQ.createTopic("thread1");

        Message msg;
        msg.content="thread1";

        Message msg2;
        msg2.content="thread1";

        msgQ.push("thread1", msg);
        this_thread::sleep_for(chrono::milliseconds(100));
        msgQ.push("thread1", msg2); });

    thread t2([&]()
              {
        msgQ.createTopic("thread1");
        size_t offset=0;

        this_thread::sleep_for(chrono::milliseconds(2000));

        for(auto m:msgQ.read("thread1",offset,5)){
            cout<<m.content<<endl;
        } });

    t1.join();
    t2.join();
}