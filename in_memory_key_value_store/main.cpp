#include "bits/stdc++.h"

using namespace std;

#define NA "N/A"

using TimePoint = chrono::steady_clock::time_point;
using Duration = chrono::steady_clock::duration;

struct Entry
{
    string value;
    TimePoint expiry;
    multimap<TimePoint, string>::iterator expirtIt;
};

class KeyValueStore
{
private:
    map<string, Entry> mp;
    multimap<TimePoint, string> keysExpiryTimes;

    mutex mtx;
    condition_variable cv;

    thread cleanupThread;

    bool stop = false;

    void erase(string key)
    {
        auto it = mp.find(key);
        if (it == mp.end())
            return;

        keysExpiryTimes.erase(it->second.expirtIt);
        mp.erase(key);
    }

    void passive_cleanup()
    {
        unique_lock<mutex> lock(mtx);

        while (!stop)
        {
            if (keysExpiryTimes.empty())
                cv.wait(lock, [this]
                        { return stop || !keysExpiryTimes.empty(); });
            else
            {
                auto nextExpiry = keysExpiryTimes.begin()->first;
                cv.wait_until(lock, nextExpiry, [this, nextExpiry]
                              { return stop ||
                                       keysExpiryTimes.empty() ||
                                       keysExpiryTimes.begin()->first != nextExpiry; });
            }

            if (stop)
                break;

            auto now = chrono::steady_clock::now();

            while (!keysExpiryTimes.empty() &&
                   keysExpiryTimes.begin()->first <= now)
            {
                erase(keysExpiryTimes.begin()->second);
            }
        }
    }

public:
    KeyValueStore()
    {
        cleanupThread = thread(&KeyValueStore::passive_cleanup, this);
    }

    ~KeyValueStore()
    {
        {
            lock_guard<mutex> lock(mtx);
            stop = true;
        }

        cv.notify_all();

        if (cleanupThread.joinable())
            cleanupThread.join();
    }

    void set(string key, string value, Duration duration)
    {

        lock_guard<mutex> lock(mtx);
        erase(key);
        auto exp = chrono::steady_clock::now() + duration;
        bool notify = keysExpiryTimes.empty() || exp < keysExpiryTimes.begin()->first;

        auto it = keysExpiryTimes.insert({exp, key});
        mp[key] = {value, exp, it};

        if (notify)
            cv.notify_one();
    }

    string get(string key)
    {
        lock_guard<mutex> lock(mtx);
        auto now = chrono::steady_clock::now();
        auto it = mp.find(key);
        if (it == mp.end() or it->second.expiry <= now)
            return NA;
        return it->second.value;
    }
};

int main()
{
    KeyValueStore kvStore;

    auto f = [&]()
    {
        int k = 5;
        while (k--)
        {
            kvStore.set("test" + to_string(k + 1), to_string(k + 1), chrono::seconds(k + 1));
            this_thread::sleep_for(chrono::seconds(1));
        }
    };

    thread t1(f);

    while (true)
    {
        this_thread::sleep_for(chrono::seconds(1));
        cout << kvStore.get("test5") << endl;
        cout << kvStore.get("test1") << endl;
    }
}