#include "bits/stdc++.h"

using namespace std;

#pragma once

enum Size
{
    SMALL = 0,
    MEDIUM = 1,
    LARGE = 2
};

class Vehicle
{
private:
    static int nextId;

public:
    int vehicleId;
    Size vehicleSize;

    static Vehicle *newVehicle(Size sz)
    {
        Vehicle *vehicle = new Vehicle();
        vehicle->vehicleId = nextId++;
        vehicle->vehicleSize = sz;

        return vehicle;
    }
};

class ParkingSpot
{
private:
    static int nextId;

    bool canFitVehicle(Vehicle *vehicle) const
    {
        return vehicle->vehicleSize <= spotSize;
    }

public:
    int spotId;
    Size spotSize;

    Vehicle *parkedVehicle;

    mutex mtx;

    static ParkingSpot *newSpot(Size sz)
    {
        ParkingSpot *spot = new ParkingSpot();
        spot->spotId = nextId++;
        spot->spotSize = sz;

        return spot;
    }

    bool tryPark(Vehicle *vehicle)
    {
        if (mtx.try_lock())
        {
            if (parkedVehicle == nullptr and canFitVehicle(vehicle))
            {
                parkedVehicle = vehicle;
                mtx.unlock();
                return true;
            }
            else
            {
                mtx.unlock();
                return false;
            }
        }
        else
            return false;
    }

    Vehicle *unpark()
    {
        lock_guard<mutex> lock(mtx);
        auto vehicle = parkedVehicle;
        parkedVehicle = nullptr;
        return vehicle;
    }
};

class ParkingFloor
{
private:
    static int nextId;

public:
    int floorId;
    vector<ParkingSpot *> parkingSpots;

    static ParkingFloor *newFloor(vector<ParkingSpot *> spots)
    {
        ParkingFloor *parkingFloor = new ParkingFloor();
        parkingFloor->floorId = nextId++;
        parkingFloor->parkingSpots = spots;
        return parkingFloor;
    }
};

class Ticket
{
private:
    static int nextId;

public:
    int ticketId;
    ParkingSpot *parkingSpot;
    ParkingFloor *parkingFloor;
    int startTime;
    int endTime;
};

int ParkingSpot::nextId = 1;
int ParkingFloor::nextId = 1;
int Ticket::nextId = 1;
int Vehicle::nextId = 1;