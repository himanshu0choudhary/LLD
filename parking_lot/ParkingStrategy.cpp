#include "bits/stdc++.h"
#include "DataClasses.cpp"

using namespace std;

#pragma once

class ParkingSpotRequest
{
public:
    Vehicle *vehicle;
};

class ParkingStrategy
{
public:
    virtual int alloteParkingSpot(ParkingSpotRequest parkingSpotRequest, vector<ParkingFloor *> &parkingFloorst) = 0;
    virtual Vehicle *removeAllocation(int spotId, vector<ParkingFloor *> &parkingFloors) = 0;
};

class SimpleParkingStrategy : public ParkingStrategy
{
public:
    int alloteParkingSpot(ParkingSpotRequest parkingSpotRequest, vector<ParkingFloor *> &parkingFloors)
    {
        for (auto floor : parkingFloors)
        {
            for (auto spot : floor->parkingSpots)
            {
                if (spot->tryPark(parkingSpotRequest.vehicle))
                    return spot->spotId;
            }
        }
        return -1;
    }

    Vehicle *removeAllocation(int spotId, vector<ParkingFloor *> &parkingFloors)
    {
        for (auto floor : parkingFloors)
        {
            for (auto spot : floor->parkingSpots)
            {
                if (spotId == spot->spotId)
                {
                    return spot->unpark();
                }
            }
        }
        return nullptr;
    }
};