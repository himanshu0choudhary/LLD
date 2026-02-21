#include "bits/stdc++.h"
#include "Song.cpp"

using namespace std;

#pragma once

class SongManager
{
private:
    unordered_map<int, Song *> songs;

public:
    int addSong(string songTitle)
    {
        Song *song = Song::newSong(songTitle);
        songs[song->songId] = song;
        return song->songId;
    }

    Song *getSong(int songId)
    {
        if (songs.find(songId) == songs.end())
            return nullptr;

        return songs[songId];
    }
};