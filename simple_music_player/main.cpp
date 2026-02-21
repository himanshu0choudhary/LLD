#include "bits/stdc++.h"
#include "SongManager.cpp"
#include "SongPlayer.cpp"

using namespace std;

int main()
{
    shared_ptr<SongManager> songManager = make_shared<SongManager>();
    shared_ptr<SongPlayer> songPlayer = make_shared<SongPlayer>(songManager);

    songManager->addSong("1");
    songManager->addSong("2");
    songManager->addSong("3");
    songManager->addSong("4");

    songPlayer->playSong(1,1);
    songPlayer->playSong(1,2);
    songPlayer->playSong(1,3);
    songPlayer->playSong(3,2);
    songPlayer->playSong(2,2);
    songPlayer->playSong(2,1);

    songPlayer->last3Song(2);
    songPlayer->printMostPlayed();
}