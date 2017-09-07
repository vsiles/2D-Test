#include <string>
#include <iostream>

using namespace std;

static unsigned char decode(char c)
{
    if ((c >= 'A') && (c <= 'Z'))
        return (c - 'A');

    if ((c >= 'a') && (c <= 'z'))
        return 26 + (c - 'a');

    if ((c >= '0') && (c <= '9'))
        return 52 + (c - '0');

    switch (c) {
        case '+': return 62;
        case '/': return 63;
        case '=': return 64;
        default: break;
    }

    cerr << "base64: decode(" << c << ") illegal call" << endl;
    return (unsigned char)-1;
}

static size_t process(unsigned char in[4], unsigned char out[3])
{
    out[0] = (in[0] & 0x3fU) << 2;
    out[0] |= (in[1] >> 4) & 0x3U;

    if (in[2] < 64) {
        out[1] = (in[1] & 0xfU) << 4;
        out[1] |= (in[2] >> 2) & 0xfU;

        if (in[3] < 64) {
            out[2] = (in[2] & 0x3U) << 6;
            out[2] |= in[3] & 0x3fU;
        } else {
            return 2;
        }
    } else {
        return 1;
    }

    return 3;
}

int b64decode(const string &data, unsigned char **decoded_data, size_t *length)
{
    size_t len = data.length();
    if (decoded_data == NULL || length == NULL || *decoded_data != NULL) {
        cerr << "base64: illegal call" << endl;
        return 1;
    }

    if ((len % 4) != 0) {
        cerr << "base64: input string is not of valid length: " << len << endl;
        return 1;
    }

    size_t max_length = 3 * (len / 4);
    unsigned char *bytes = new unsigned char[max_length];
    if (bytes == NULL) {
        cerr << "base64: can't allocate output buffer of size " << max_length << endl;
        return 1;
    }

    *length = 0;
    *decoded_data = bytes;

    unsigned char in[4] = { 0 };
    unsigned char out[3] = { 0 };
    size_t pos = 0, out_pos = 0;

    while (len > 0) {
        in[0] = decode(data[pos + 0]);
        if (in[0] == (unsigned char)-1) {
            cerr << "base64: decoding failed" << endl;
            delete [] bytes;
            return 1;
        }
        in[1] = decode(data[pos + 1]);
        if (in[1] == (unsigned char)-1) {
            cerr << "base64: decoding failed" << endl;
            delete [] bytes;
            return 1;
        }
        in[2] = decode(data[pos + 2]);
        if (in[2] == (unsigned char)-1) {
            cerr << "base64: decoding failed" << endl;
            delete [] bytes;
            return 1;
        }
        in[3] = decode(data[pos + 3]);
        if (in[3] == (unsigned char)-1) {
            cerr << "base64: decoding failed" << endl;
            delete [] bytes;
            return 1;
        }

        out[0] = out[1] = out[2] = 0;
        *length += process(in, out);
        bytes[out_pos + 0] = out[0];
        bytes[out_pos + 1] = out[1];
        bytes[out_pos + 2] = out[2];

        pos += 4;
        len -= 4;
        out_pos += 3;
    }

    return 0;
}
