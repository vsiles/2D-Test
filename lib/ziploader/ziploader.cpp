#include <fstream>
#include <string>
#include <cassert>
#include <cstdint>
#include "errors.hpp"
#include "ziploader.hpp"

using namespace std;

#define LFH_MAGIC       UINT32_C(0x04034b50)
#define EOCD_MAGIC      UINT32_C(0x06054b50)
#define CDFH_MAGIC      UINT32_C(0x02014b50)

#define CMT_MAX         ((size_t)(1 << 16))

#define EOCD_SIZE       ((size_t)22U)
#define CDFH_SIZE       ((size_t)46U)

static void zipError(const string &filename, const string &message)
{
    string err = "While loading '" + filename + "': " + message;
    logError("ZIP", err);
}

ZipFile::ZipFile()
{
    num_files = 0;
    files_info.clear();
}

bool ZipFile::Init(const string &path)
{
    ifstream fp;

    fp.open(path.c_str(), ios_base::in | ios_base::binary);

    if (!fp.is_open() || !fp.good()) {
        zipError(path, "Open error");
        return false;
    }

    /* set exceptions to catch all possible errors */
    fp.exceptions(ifstream::failbit | ifstream::badbit | ifstream::eofbit);
    bool ret;
    try {
        ret = InitScan(fp, path);
    } catch (ifstream::failure e) {
        string err = "Init failure (";
        err += e.what();
        err += ")";
        zipError(path, err);
        ret = false;
    }

    fp.close();
    return ret;
}

/**
 * @throws std::ifstream::failure
 */
bool ZipFile::InitScan(ifstream &fp, const string &path)
{
    uint32_t magic;

    /* The first word is always a Local File Header signature */
    fp.read((char *)(&magic), sizeof(uint32_t));
    if (magic != LFH_MAGIC) {
        zipError(path, "Bad Magic");
        return false;
    }

    fp.seekg(0, ios_base::end);
    size_t total_size = fp.tellg();

    /* Look for the End of Central Directory. It must be in the
     * last EOCD_SIZE + CMT_MAX bytes of the file
     */
    int size = EOCD_SIZE + CMT_MAX;
    fp.seekg(-size, ios_base::end);

    unsigned char *buffer = new unsigned char[size];
    unsigned char *buffer_end = buffer + size;
    fp.read((char *)buffer, size);

    unsigned char *scan = buffer;
    uint32_t word;
    while ((scan + 4) < buffer_end) {
        word = *(uint32_t *)scan;
        if (word == EOCD_MAGIC)
            break;
        scan++;
    }

    if (word != EOCD_MAGIC) {
        zipError(path, "Can't find EOCD");
        delete [] buffer;
        return false;
    }

    unsigned char *eocd = scan;

    scan += sizeof(uint32_t);

    /* Some heavy restrictions, at the moment
     * TODO: support more of ZIP format
     */
    uint16_t nr_disk = *(uint16_t *)scan;
    scan += sizeof(uint16_t);
    assert(nr_disk == 0);
    uint16_t disk_start = *(uint16_t *)scan;
    scan += sizeof(uint16_t);
    assert(disk_start == 0);
    uint16_t nr_rec_on_disk = *(uint16_t *)scan;
    scan += sizeof(uint16_t);
    num_files = *(uint16_t *)scan;
    log(string("Number of files in the zip: ") + to_string(num_files));
    scan += sizeof(uint16_t);
    assert(num_files = nr_rec_on_disk);
    uint32_t cd_size = *(uint32_t *)scan;
    scan += sizeof(uint32_t);
    uint32_t cd_offset = *(uint32_t *)scan;
    scan += sizeof(uint32_t);
    uint64_t comment_size = *(uint16_t *)scan;

    delete [] buffer;

    /* Sanity check */
    assert(cd_offset + cd_size <= total_size);
    assert(eocd + EOCD_SIZE + comment_size == buffer_end);

    files_info.resize(num_files);
    fp.seekg(cd_offset, ios_base::beg);

    size_t bytes = 0;
    for (size_t nr = 0; nr < num_files; nr++) {
        ZipFileInfo *info = &(files_info[nr]);

        buffer = new unsigned char[CDFH_SIZE];
        buffer_end = buffer + CDFH_SIZE;

        size_t start_pos = fp.tellg();
        fp.read((char *)buffer, CDFH_SIZE);

        scan = buffer;
        word = *(uint32_t *)scan;
        if (word != CDFH_MAGIC) {
            zipError(path, string("Can't find CDFH - ") + to_string(nr));
            delete [] buffer;
            return false;
        }
        scan += sizeof(uint32_t);

        scan += sizeof(uint16_t); // skip "Version made by"
        scan += sizeof(uint16_t); // skip "Version needed to extract (minimum)
        scan += sizeof(uint16_t); // skip "General purpose bit flag"
        uint16_t compression_method = *(uint16_t *)scan;
        scan += sizeof(uint16_t);
        if (compression_method != 0) {
            info->compressed = true;
            assert(compression_method == 8 /* deflate */);
        } else {
            info->compressed = false;
        }
        scan += sizeof(uint16_t); // skip "File last modification time"
        scan += sizeof(uint16_t); // skip "File last modification date"
        scan += sizeof(uint32_t); // skip "CRC32"
        info->compressed_size = *(uint32_t *)scan;
        scan += sizeof(uint32_t);
        info->size = *(uint32_t *)scan;
        scan += sizeof(uint32_t);
        uint16_t file_name_length = *(uint16_t *)scan;
        assert(file_name_length != 0);
        scan += sizeof(uint16_t);
        uint16_t extra_field_length = *(uint16_t *)scan;
        scan += sizeof(uint16_t);
        uint16_t comment_length = *(uint16_t *)scan;
        scan += sizeof(uint16_t);
        disk_start = *(uint16_t *)scan;
        scan += sizeof(uint16_t);
        assert(disk_start == 0);
        scan += sizeof(uint16_t); // skip "internal file attributes"
        scan += sizeof(uint32_t); // skip "external file attributes"
        info->offset = *(uint32_t *)scan;
        scan += sizeof(uint32_t);
        assert(scan == buffer_end);

        delete [] buffer;
        info->name.resize(file_name_length);
        fp.read((char *)&(info->name[0]), file_name_length);
        fp.seekg(extra_field_length + comment_length, ios_base::cur);

        size_t end_pos = fp.tellg();
        bytes += end_pos - start_pos;

        if (info->size != 0)
            cout << files_info[nr] << endl;
    }

    assert(bytes == cd_size);

    return true;
}
