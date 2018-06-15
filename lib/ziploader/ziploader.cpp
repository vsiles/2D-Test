#include <fstream>
#include <string>
#include <cassert>
#include <cstdint>
#include <vector>
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

static uint16_t buf2u16(const vector<unsigned char>::iterator &it)
{
    unsigned char c0 = *it;
    unsigned char c1 = *(it + 1);
    uint16_t ret = c0;
    ret |= ((uint16_t)c1) << 8;
    return ret;
}

static uint32_t buf2u32(const vector<unsigned char>::iterator &it)
{
    unsigned char c0 = *it;
    unsigned char c1 = *(it + 1);
    unsigned char c2 = *(it + 2);
    unsigned char c3 = *(it + 3);
    uint32_t ret = c0;
    ret |= ((uint32_t)c1) << 8;
    ret |= ((uint32_t)c2) << 16;
    ret |= ((uint32_t)c3) << 24;
    return ret;
}

static uint16_t read_u16(vector<unsigned char>::iterator &it)
{
    uint16_t val = buf2u16(it);
    it += sizeof(uint16_t);
    return val;
}

static uint32_t read_u32(vector<unsigned char>::iterator &it)
{
    uint32_t val = buf2u32(it);
    it += sizeof(uint32_t);
    return val;
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

    vector<unsigned char> buffer(size);
    fp.read((char *)&(buffer[0]), size);

    vector<unsigned char>::iterator scan = buffer.begin();
    vector<unsigned char>::iterator last_possible = buffer.end() - 4;
    uint32_t word;
    while (scan != last_possible) {
        word = buf2u32(scan);
        if (word == EOCD_MAGIC)
            break;
        scan++;
    }

    if (word != EOCD_MAGIC) {
        zipError(path, "Can't find EOCD");
        return false;
    }
    scan += sizeof(uint32_t);

    /* Some heavy restrictions, at the moment
     * TODO: support more of ZIP format
     */
    uint16_t nr_disk = read_u16(scan);
    assert(nr_disk == 0);
    uint16_t disk_start = read_u16(scan);
    assert(disk_start == 0);
    uint16_t nr_rec_on_disk = read_u16(scan);
    num_files = read_u16(scan);
    log(string("Number of files in the zip: ") + to_string(num_files));
    assert(num_files == nr_rec_on_disk);
    uint32_t cd_size = read_u32(scan);
    uint32_t cd_offset = read_u32(scan);
    uint64_t comment_size = read_u16(scan);
    scan += comment_size;
    assert(scan == buffer.end());

    buffer.clear();

    /* Sanity check */
    assert(cd_offset + cd_size <= total_size);

    files_info.resize(num_files);
    fp.seekg(cd_offset, ios_base::beg);

    size_t bytes = 0;
    buffer.resize(CDFH_SIZE);

    for (size_t nr = 0; nr < num_files; nr++) {
        log(string("Parsing file ") + to_string(nr));
        ZipFileInfo *info = &(files_info[nr]);


        size_t start_pos = fp.tellg();
        fp.read((char *)&(buffer[0]), CDFH_SIZE);

        scan = buffer.begin();
        word = read_u32(scan);
        if (word != CDFH_MAGIC) {
            zipError(path, string("Can't find CDFH - ") + to_string(nr));
            return false;
        }

        scan += sizeof(uint16_t); // skip "Version made by"
        scan += sizeof(uint16_t); // skip "Version needed to extract (minimum)
        scan += sizeof(uint16_t); // skip "General purpose bit flag"
        uint16_t compression_method = read_u16(scan);
        if (compression_method != 0) {
            info->compressed = true;
            assert(compression_method == 8 /* deflate */);
        } else {
            info->compressed = false;
        }
        scan += sizeof(uint16_t); // skip "File last modification time"
        scan += sizeof(uint16_t); // skip "File last modification date"
        scan += sizeof(uint32_t); // skip "CRC32"
        info->compressed_size = read_u32(scan);
        info->size = read_u32(scan);
        uint16_t file_name_length = read_u16(scan);
        assert(file_name_length != 0);
        uint16_t extra_field_length = read_u16(scan);
        uint16_t comment_length = read_u16(scan);
        disk_start = read_u16(scan);
        assert(disk_start == 0);
        scan += sizeof(uint16_t); // skip "internal file attributes"
        scan += sizeof(uint32_t); // skip "external file attributes"
        info->offset = read_u32(scan);
        assert(scan == buffer.end());

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
