#include "bits/stdc++.h"
#include "PlatformManagerStrategy.cpp"
#include "Platform.cpp"

using namespace std;

#pragma once

class PlatformManager
{
private:
    PlatformManagementStrategy *platformManagementStrategy;

public:
    PlatformManager(PlatformManagementStrategy *platformManagementStrategy) : platformManagementStrategy(platformManagementStrategy) {}

    void assign(int trainId, int from, int to)
    {
        PlatformAssignmentRequest platformAssignmentRequest;
        platformAssignmentRequest.from = from;
        platformAssignmentRequest.to = to;
        platformAssignmentRequest.trainId = trainId;

        PlatformAssignmentResponse platformAssignmentResponse = platformManagementStrategy->assignPlatform(platformAssignmentRequest);

        if (platformAssignmentResponse.status)
        {
            cout << "Platform:" << platformAssignmentResponse.platformId
                 << " was assigned to train:" << trainId
                 << " from:" << from << " to:" << to << endl;
        }
        else
        {
            cout << "Unable to assign Platform" << endl;
        }
    }

    void remove(int trainId)
    {
        PlatformRemoveAssignmentRequest platformRemoveAssignmentRequest;
        platformRemoveAssignmentRequest.trainId = trainId;

        PlatformRemoveAssignmentResponse platformRemoveAssignmentResponse = platformManagementStrategy->removeAssignment(platformRemoveAssignmentRequest);

        if (platformRemoveAssignmentResponse.status)
        {
            cout << "Platform: " << platformRemoveAssignmentResponse.platformId
                 << " was removed from train:" << trainId
                 << " from:" << platformRemoveAssignmentResponse.from << " to:" << platformRemoveAssignmentResponse.to << endl;
        }
        else
        {
            cout << "Unable to remove Platform" << endl;
        }
    }
};