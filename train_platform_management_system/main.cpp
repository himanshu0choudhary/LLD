#include "bits/stdc++.h"
#include "PlatformManagerStrategy.cpp"
#include "PlatformManager.cpp"

using namespace std;

int main()
{
    PlatformManagementStrategy *platformManagementStrategy = new SimplePlatformManagementStrategy(3);

    PlatformManager platformManager = PlatformManager(platformManagementStrategy);

    platformManager.assign(1, 1, 20);
    platformManager.assign(1, 1, 21);

    platformManager.assign(2, 5, 100);
    platformManager.assign(3, 25, 100);

    platformManager.remove(1);
    platformManager.remove(3);
}