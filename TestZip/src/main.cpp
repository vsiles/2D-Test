#include <iostream>
#include <string>

using namespace std;

#include "errors.hpp"
#include "resource.hpp"
#include "ziploader.hpp"

int main(void)
{
    try {
        ZipFile zip;

        string resPath = getResourcePath();
        if (zip.Init(resPath + "ziptest.zip")) {
            log("ZipFile parsed correctly");
        } else {
            return -1;
        }
    } catch (bad_alloc e) {
        cerr << "Some 'new' allocation failed: " << e.what() << endl;
        return -2;
    }
    return 0;
}
