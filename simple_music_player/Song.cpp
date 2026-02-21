#include "bits/stdc++.h"

using namespace std;

#pragma once

class Song
{
private:
    static int nextId;

public:
    int songId;
    string title;
    int duration;

    static Song *newSong(string title)
    {
        Song *song = new Song();
        song->songId = nextId++;
        song->title = title;
        return song;
    }
};

int Song::nextId = 1;