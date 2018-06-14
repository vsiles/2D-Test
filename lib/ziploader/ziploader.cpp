#include <fstream>
#include <string>
#include "errors.hpp"
#include "ziploader.hpp"

using namespace std;

ZipFile::ZipFile()
{
    num_files = 0;
}

ZipFile::~ZipFile()
{

}

bool ZipFile::Init(const string &path)
{
    ifstream fp;

    fp.open(path.c_str(), ios_base::in | ios_base::binary);

    if (!fp.is_open() && !fp.good()) {
        string err = "Can't open file '";
        err += path;
        err += "' for reading";
        logError("ZIP", err);
        return false;
    }
    
    fp.close();
    return true;
}