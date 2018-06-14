#ifndef ZIP_H_INCLUDED
#define ZIP_H_INCLUDED

#include <string>

class ZipFile
{
    private:
        size_t num_files;
    public:
        ZipFile();
        ~ZipFile();

        bool Init(const std::string &path);
};

#endif /* ZIP_H_INCLUDED */