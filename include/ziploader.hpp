#ifndef ZIP_H_INCLUDED
#define ZIP_H_INCLUDED

#include <string>
#include <fstream>
#include <iostream>
#include <vector>

class ZipFile;

class ZipFileInfo
{
    friend class ZipFile;

    private:
        std::string name;
        size_t offset;
        size_t compressed_size;
        size_t size;
        bool compressed;

    public:
        friend std::ostream &operator<<(std::ostream &output,
                                        const ZipFileInfo &info)
        {
            output << "Filename: " << info.name << std::endl;
            output << "  offset: " << info.offset << std::endl;
            output << "  size  : " << info.compressed_size << " (compressed)"
                << std::endl;
            output << "        : " << info.size << std::endl;
            return output;
        }
};

class ZipFile
{
    private:
        std::string path;
        size_t num_files;
        std::vector<ZipFileInfo> files_info;

        /**
         * @throws std::ifstream::failure
         */
        bool InitScan(std::ifstream &fp, const std::string &path);
    public:
        ZipFile();

        bool Init(const std::string &path);
        bool Find(const std::string &name, std::vector<unsigned char> &data,
                  size_t *bytes);
        void List();
};

#endif /* ZIP_H_INCLUDED */
