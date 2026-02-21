#include "bits/stdc++.h"
#include "Song.cpp"
#include "SongManager.cpp"

using namespace std;

#pragma once

class SongPlayer
{
private:
    const int MAX_HISTORY_LEN = 10;
    shared_ptr<SongManager> songManager;
    map<int, list<int>> userHistory;

    map<int, set<int>> maxPlayCount;
    map<int, set<int>> songPlayedByUsers;

    void updatePlayCount(int songId, int userId)
    {
        if (songPlayedByUsers[songId].find(userId) != songPlayedByUsers[songId].end())
            return;

        int currCnt = songPlayedByUsers[songId].size();
        songPlayedByUsers[songId].insert(userId);

        maxPlayCount[currCnt].erase(songId);
        if (maxPlayCount[currCnt].empty())
            maxPlayCount.erase(currCnt);
        maxPlayCount[currCnt + 1].insert(songId);
    }

public:
    SongPlayer(shared_ptr<SongManager> songManager) : songManager(songManager) {}

    void playSong(int songId, int userId)
    {
        Song *song = songManager->getSong(songId);
        if (song == nullptr)
            return;

        userHistory[userId].push_back(songId);
        updatePlayCount(songId, userId);
    }

    void printMostPlayed()
    {
        cout << "Most Played:" << endl;
        for (auto it = maxPlayCount.rbegin(); it != maxPlayCount.rend(); ++it)
        {
            cout << it->first << " -> ";
            for (auto song : it->second)
                cout << song << " ";
            cout << endl;
        }
    }

    void last3Song(int userId)
    {
        int k = 3;

        cout << "Last 3 Songs -> ";
        for (auto it = userHistory[userId].rbegin(); it != userHistory[userId].rend() and (--k) < 3; it++)
        {
            cout << (*it) << " ";
        }
        cout << endl;
    }
};