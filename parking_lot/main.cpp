#include "bits/stdc++.h"
#include "DataClasses.cpp"
#include "ParkingStrategy.cpp"
#include "ParkingLot.cpp"

using namespace std;

int main()
{
    ParkingFloor *floor1 = ParkingFloor::newFloor({
        ParkingSpot::newSpot(Size::SMALL),
        ParkingSpot::newSpot(Size::MEDIUM),
        ParkingSpot::newSpot(Size::MEDIUM),
        ParkingSpot::newSpot(Size::LARGE),
    });

    ParkingFloor *floor2 = ParkingFloor::newFloor({
        ParkingSpot::newSpot(Size::SMALL),
        ParkingSpot::newSpot(Size::SMALL),
        ParkingSpot::newSpot(Size::SMALL),
        ParkingSpot::newSpot(Size::MEDIUM),
    });

    Vehicle *vehicle1 = Vehicle::newVehicle(Size::LARGE);
    Vehicle *vehicle2 = Vehicle::newVehicle(Size::MEDIUM);

    ParkingLot *parkingLot = new ParkingLot({floor1, floor2}, make_shared<SimpleParkingStrategy>());

    auto f = [&]()
    {
        for (int i = 0; i < 10; i++)
        {
            int spotId = parkingLot->alloteParkingSpot(vehicle1);
            cout << "Found spot: " << spotId << " in thread: " << this_thread::get_id() << endl;
            this_thread::sleep_for(chrono::seconds(1));
            parkingLot->removeAllocation(spotId);
            cout << "Removed spot: " << spotId << " in thread: " << this_thread::get_id() << endl;
        }
    };

    thread t1(f), t2(f);
    t1.join();
    t2.join();
}