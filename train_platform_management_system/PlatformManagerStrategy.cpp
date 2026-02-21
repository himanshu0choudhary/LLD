#include "bits/stdc++.h"
#include "Platform.cpp"

using namespace std;

#pragma once

// Assign
struct PlatformAssignmentRequest
{
    int trainId;
    int from;
    int to;
};

struct PlatformAssignmentResponse
{
    int trainId;
    int from;
    int to;
    int platformId;
    bool status;
};

// Remove
struct PlatformRemoveAssignmentRequest
{
    int trainId;
};

struct PlatformRemoveAssignmentResponse
{
    int from;
    int to;
    int platformId;
    bool status;
};

class PlatformManagementStrategy
{
public:
    virtual PlatformAssignmentResponse assignPlatform(PlatformAssignmentRequest platformAssignmentRequest) = 0;
    virtual PlatformRemoveAssignmentResponse removeAssignment(PlatformRemoveAssignmentRequest platformRemoveAssignmentRequest) = 0;
};

class SimplePlatformManagementStrategy : public PlatformManagementStrategy
{
private:
    const int MAX_TIME = 10000;

    int numPlatforms;
    vector<vector<int>> allotment;
    vector<Platform> platforms;

    map<int, int> trainToPlatformMap;

public:
    SimplePlatformManagementStrategy(int numPlatforms) : numPlatforms(numPlatforms)
    {
        for (int i = 0; i < numPlatforms; i++)
        {
            platforms.push_back(Platform(i));
        }

        allotment = vector<vector<int>>(numPlatforms, vector<int>(MAX_TIME, -1));
    }

    PlatformAssignmentResponse assignPlatform(PlatformAssignmentRequest platformAssignmentRequest)
    {
        int trainId = platformAssignmentRequest.trainId;
        int from = platformAssignmentRequest.from, to = platformAssignmentRequest.to;

        PlatformAssignmentResponse response;

        if (trainToPlatformMap.find(trainId) != trainToPlatformMap.end())
        {
            response.status = false;
            return response;
        }

        if (from > to or from <= 0 or to >= MAX_TIME)
        {
            response.status = false;
            return response;
        }

        for (int i = 0; i < numPlatforms; i++)
        {
            bool isAvailable = true;

            for (int j = from; j <= to; j++)
            {
                if (allotment[i][j] != -1)
                {
                    isAvailable = false;
                    break;
                }
            }

            if (isAvailable)
            {
                trainToPlatformMap[trainId] = i;
                for (int j = from; j <= to; j++)
                    allotment[i][j] = trainId;

                response.from = from;
                response.to = to;
                response.platformId = i;
                response.trainId = trainId;
                response.status = true;
                return response;
            }
        }

        response.status = false;
        return response;
    }

    PlatformRemoveAssignmentResponse removeAssignment(PlatformRemoveAssignmentRequest platformRemoveAssignmentRequest)
    {
        int trainId = platformRemoveAssignmentRequest.trainId;

        PlatformRemoveAssignmentResponse platformRemoveAssignmentResponse;

        if (trainToPlatformMap.find(trainId) == trainToPlatformMap.end())
        {
            platformRemoveAssignmentResponse.status = false;
            return platformRemoveAssignmentResponse;
        }

        int platformId = trainToPlatformMap[trainId];
        platformRemoveAssignmentResponse.platformId = platformId;
        trainToPlatformMap.erase(trainId);

        int ind = 0;

        while (ind < MAX_TIME and allotment[platformId][ind] != trainId)
            ind++;
        platformRemoveAssignmentResponse.from = ind;

        while (ind < MAX_TIME and allotment[platformId][ind] == trainId)
        {
            allotment[platformId][ind++] = -1;
        }
        platformRemoveAssignmentResponse.to = ind - 1;

        platformRemoveAssignmentResponse.status = true;
        return platformRemoveAssignmentResponse;
    }
};