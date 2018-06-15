#include <fstream>
#include <string>
#include <cassert>
#include <cstdint>
#include <cstring> // temp, memcpy
#include <vector>
#include <zlib.h>
#include "errors.hpp"
#include "ziploader.hpp"

using namespace std;

#define LFH_MAGIC       UINT32_C(0x04034b50)
#define EOCD_MAGIC      UINT32_C(0x06054b50)
#define CDFH_MAGIC      UINT32_C(0x02014b50)

#define CMT_MAX         ((size_t)(1 << 16))

#define LFH_SIZE        ((size_t)30U)
#define EOCD_SIZE       ((size_t)22U)
#define CDFH_SIZE       ((size_t)46U)

#define LFH_FLAGS_DESCR (UINT16_C(1) << 3)

static void zipError(const string &filename, const string &message)
{
    string err = "While loading '" + filename + "': " + message;
    logError("ZIP", err);
}

ZipFile::ZipFile()
{
    num_files = 0;
    files_info.clear();
    path.clear();
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

    this->path = path;
    return ret;
}

static uint16_t it2u16(const vector<unsigned char>::iterator &it)
{
    unsigned char c0 = *it;
    unsigned char c1 = *(it + 1);
    uint16_t ret = static_cast<uint16_t>(c0);
    ret |= static_cast<uint16_t>(c1) << 8;
    return ret;
}

static uint32_t byte2u32(unsigned char c0, unsigned char c1, unsigned char c2,
                         unsigned char c3)
{
    uint32_t ret = c0;
    ret |= static_cast<uint32_t>(c1) << 8;
    ret |= static_cast<uint32_t>(c2) << 16;
    ret |= static_cast<uint32_t>(c3) << 24;
    return ret;
}

static uint32_t it2u32(const vector<unsigned char>::iterator &it)
{
    unsigned char c0 = *it;
    unsigned char c1 = *(it + 1);
    unsigned char c2 = *(it + 2);
    unsigned char c3 = *(it + 3);
    return byte2u32(c0, c1, c2, c3);
}

static uint16_t read_u16(vector<unsigned char>::iterator &it)
{
    uint16_t val = it2u16(it);
    it += sizeof(uint16_t);
    return val;
}

static uint32_t read_u32(vector<unsigned char>::iterator &it)
{
    uint32_t val = it2u32(it);
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
    fp.read(reinterpret_cast<char *>(&magic), sizeof(uint32_t));
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
    fp.read(reinterpret_cast<char *>(&(buffer[0])), size);

    vector<unsigned char>::iterator scan = buffer.begin();
    vector<unsigned char>::iterator last_possible = buffer.end() - 4;
    uint32_t word;
    while (scan != last_possible) {
        word = it2u32(scan);
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
        fp.read(reinterpret_cast<char *>(&(buffer[0])), CDFH_SIZE);

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
        /* This is the offset of the Local File Header. We'll update it to the actual data */
        info->offset = read_u32(scan);
        assert(scan == buffer.end());

        info->name.resize(file_name_length);
        fp.read(static_cast<char *>(&(info->name[0])), file_name_length);
        log(info->name);
        fp.seekg(extra_field_length + comment_length, ios_base::cur);

        size_t end_pos = fp.tellg();
        bytes += end_pos - start_pos;
    }
    assert(bytes == cd_size);

    /* Now that the Central directory is parsed, update the offset of each
     * non empty file
     */
    buffer.resize(LFH_SIZE);
    for (size_t nr = 0; nr < num_files; nr++) {
        ZipFileInfo *info = &(files_info[nr]);
        if (info->size == 0)
            continue;
        fp.seekg(info->offset, ios_base::beg);
        fp.read(reinterpret_cast<char *>(&(buffer[0])), LFH_SIZE);
        scan = buffer.begin();
        word = read_u32(scan);
        if (word != LFH_MAGIC) {
            zipError(path, string("Can't locate data for file ") +
                     to_string(nr));
            return false;
        }
        scan += sizeof(uint16_t); // skip "Version needed to extract (minimum)
        uint16_t flags = read_u16(scan);
        assert((flags & LFH_FLAGS_DESCR) == 0); // No support for data descr
        uint16_t compression_method = read_u16(scan);
        if (info->compressed)
            assert(compression_method == 8); // deflate only
        scan += sizeof(uint16_t); // skip "File last modification time"
        scan += sizeof(uint16_t); // skip "File last modification date"
        scan += sizeof(uint32_t); // skip "CRC32"
        uint32_t csize = read_u32(scan);
        uint32_t size = read_u32(scan);
        assert(csize == info->compressed_size);
        assert(size == info->size);
        uint16_t file_name_length = read_u16(scan);
        uint16_t extra_field_length = read_u16(scan);
        assert(scan == buffer.end());

        fp.seekg(file_name_length + extra_field_length, ios_base::cur);
        info->offset = fp.tellg();
    }

    return true;
}

#define INFLATE_CHUNK ((size_t)(16 * 1024))

static int raw_inflate(vector<unsigned char> &data, ifstream &fp,
                       size_t compressed_size)
{
    int ret;
    size_t have;
    z_stream strm;
    static vector<unsigned char>in(INFLATE_CHUNK);

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    /* ZIP use raw DEFLATE */
    ret = inflateInit2(&strm, -15);
    if (ret != Z_OK)
        return ret;

    /* decompress until deflate stream ends or end of file */
    size_t bytes_left = compressed_size;
    vector<unsigned char>::iterator cur = data.begin();
    do {
        size_t sz = (bytes_left < INFLATE_CHUNK)?bytes_left:INFLATE_CHUNK;
        fill(in.begin(), in.end(), 0);
        fp.read(reinterpret_cast<char *>(&(in[0])), sz);
        if (!fp.good()) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        strm.avail_in = sz;
        if (strm.avail_in == 0)
            break;
        strm.next_in = static_cast<unsigned char *>(&(in[0]));;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = INFLATE_CHUNK;
            strm.next_out = static_cast<unsigned char *>(&(*cur));

            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
            }

            have = INFLATE_CHUNK - strm.avail_out;
            cur += have;
        } while (strm.avail_out == 0);
        bytes_left -= sz;

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);
    assert(bytes_left == 0);

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

bool ZipFile::Find(const string &name, vector<unsigned char> &data,
                   size_t *bytes)
{
    if (num_files == 0)
        return false;

    vector<ZipFileInfo>::iterator it = files_info.begin();
    while (it != files_info.end()) {
        if (name == (*it).name) {
            cout << *it << endl;
            break;
        }
        it++;
    }

    if (it == files_info.end())
        return false;

    size_t csize = (*it).compressed_size;
    size_t size = (*it).size;

    ifstream fp;
    fp.open(path.c_str(), ios_base::in | ios_base::binary);

    if (!fp.is_open() || !fp.good()) {
        zipError(path, "Open error");
        return false;
    }

    data.resize(size);

    int ret;
    fp.seekg((*it).offset, ios_base::beg);
    if (!fp.good()) {
        zipError(path, "Can't find compressed data");
        fp.close();
        return false;
    }

    ret = raw_inflate(data, fp, csize);
    fp.close();

    switch (ret) {
        case Z_OK:
            cout << "Decompressing successful" << endl;
            ret = 0;
            break;
        case Z_MEM_ERROR:
            cerr << "Decompressing ran out of memory" << endl;
            ret = 1;
            break;
        case Z_BUF_ERROR:
            cerr << "Decompressing buffer not large enough" << endl;
            ret = 1;
            break;
        case Z_DATA_ERROR:
            cerr << "Decompressing: input data corruption" << endl;
            ret = 1;
            break;
        default:
            cerr << "Decompressing: unknown error " << ret << endl;
            ret = 1;
            break;
    }
    if (ret != 0) {
        zipError((*it).name, "Uncompress failed");
        return false;
    }

    *bytes = size;
    return true;
}
