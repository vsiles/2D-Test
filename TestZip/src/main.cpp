#include <iostream>
#include <string>
#include <vector>

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
        vector<unsigned char> data;
        size_t bytes = 0;

        string filename("resources/README.md");
        if (!zip.Find(filename, data, &bytes)) {
            logError("XXX", filename + " is missing !");
            return -1;
        }

        for (size_t n = 0; n < bytes; n++) {
            cout << data[n];
        }
        cout << endl << "Success !" << endl;
    } catch (bad_alloc e) {
        cerr << "Some 'new' allocation failed: " << e.what() << endl;
        return -2;
    }
    return 0;
}
