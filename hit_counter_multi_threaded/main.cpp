#include "bits/stdc++.h"

using namespace std;

class HitCounter
{
private:
    struct Bucket
    {
        mutex mtx;     // Local lock for this second
        int count = 0; // No longer needs to be atomic (protected by mtx)
        long long timestamp = 0;
    };

    vector<Bucket> buckets;
    const int windowSize = 300;

public:
    HitCounter()
    {
        buckets = vector<Bucket>(300);
    }

    void recordHit(long long timestamp)
    {
        int idx = timestamp % windowSize;

        // Only lock the specific bucket for this second
        lock_guard<mutex> lock(buckets[idx].mtx);

        if (buckets[idx].timestamp != timestamp)
        {
            // If it's a new second, reset.
            // If it's an OLD timestamp arriving late, we ignore it to prevent overwriting.
            if (timestamp > buckets[idx].timestamp)
            {
                buckets[idx].timestamp = timestamp;
                buckets[idx].count = 1;
            }
        }
        else
        {
            // Same second, just increment safely
            buckets[idx].count++;
        }
    }

    int getHits(long long currentTimestamp)
    {
        int totalHits = 0;
        for (int i = 0; i < windowSize; ++i)
        {
            // We lock briefly to read the bucket's state consistently
            lock_guard<mutex> lock(buckets[i].mtx);
            if (currentTimestamp - buckets[i].timestamp < windowSize)
            {
                totalHits += buckets[i].count;
            }
        }
        return totalHits;
    }
};

int main()
{
    HitCounter hitCounter;

    auto f = [&]()
    {
        while (true)
        {
            auto now = chrono::steady_clock::now();
            int timestamp = chrono::duration_cast<chrono::seconds>(
                                now.time_since_epoch())
                                .count();
            hitCounter.recordHit(timestamp);
            this_thread::sleep_for(chrono::seconds(1));
        }
    };

    thread t1(f);
    thread t2(f);
    thread t3(f);

    while (true)
    {
        auto now = chrono::steady_clock::now();
        int timestamp = chrono::duration_cast<chrono::seconds>(
                            now.time_since_epoch())
                            .count();
        cout << hitCounter.getHits(timestamp) << endl;
        this_thread::sleep_for(chrono::seconds(1));
    }
}