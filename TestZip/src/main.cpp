#include <iostream>
#include <string>

using namespace std;

#include "resource.hpp"
#include "ziploader.hpp"

int main(void)
{
    ZipFile zip;

    string resPath = getResourcePath();
    cout << zip.Init(resPath + "ziptest.zip") << endl;
    return 0;
}