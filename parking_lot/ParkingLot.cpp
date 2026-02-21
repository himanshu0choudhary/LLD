#include "bits/stdc++.h"
#include "DataClasses.cpp"
#include "ParkingStrategy.cpp"

using namespace std;

#pragma once

class ParkingLot
{
private:
    vector<ParkingFloor *> parkingFloors;
    shared_ptr<ParkingStrategy> parkingStrategy;

public:
    ParkingLot(vector<ParkingFloor *> parkingFloors,
               shared_ptr<ParkingStrategy> parkingStrategy) : parkingFloors(parkingFloors), parkingStrategy(parkingStrategy) {}

    int alloteParkingSpot(Vehicle *vehicle)
    {
        ParkingSpotRequest request = ParkingSpotRequest();
        request.vehicle = vehicle;

        return parkingStrategy->alloteParkingSpot(request, parkingFloors);
    }

    Vehicle *removeAllocation(int spotId)
    {
        return parkingStrategy->removeAllocation(spotId, parkingFloors);
    }
};