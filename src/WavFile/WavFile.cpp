#include "WavFile.hpp"
#include "string.h"
#include <iostream>

using std::cout;
using std::string;

WavFile::WavFile(uint32_t bytes_per_sample, uint32_t samples_per_block, unsigned char *mem, bool disable_writing=false)
: disable_writing_(disable_writing), bytes_per_sample_(bytes_per_sample), samples_per_block_(samples_per_block), next_ch_(0), mem_(mem)
{
    up_shift_bits_ = 0;     // default
    if (3 == bytes_per_sample_)
    {                           // incoming 24 bit
        bytes_per_sample_ = 4;   // writing to file 32 bit
        up_shift_bits_ = 8; // shift up 8 bits to use full-scale (8 LSB will be 0)
    }
}

bool WavFile::open_write(const char *file_name, uint32_t sample_rate, uint32_t number_of_channels)
{
    if (disable_writing_)
    {
        return true;  // don't do anything
    }

    chans_ = number_of_channels;
    wav_header.num_channels = chans_;
    wav_header.sample_rate = sample_rate;
    wav_header.byte_rate = sample_rate * chans_ * bytes_per_sample_;
    wav_header.sample_alignment = chans_ * bytes_per_sample_;
    wav_header.bit_depth = 8 * bytes_per_sample_;

    bytes_per_block_ = bytes_per_sample_ * samples_per_block_ * chans_;
    if (0u == bytes_per_block_)
    {
        return false;
    }
    memset(mem_, 0, bytes_per_block_);

    fp_ = fopen(file_name,"wb");
    if (NULL == fp_)
    {
        return false;
    }

    if (sizeof(Wav_header) != fwrite(&wav_header, (size_t)1, sizeof(Wav_header), fp_))
    {
        return false;
    }
    byte_pos_ = sizeof(Wav_header);

    return true;
}

bool WavFile::fill_channel(const void *buffer, int32_t ch)
{
    if (disable_writing_)
    {
        return true;  // don't do anything
    }

    if (-1 == ch)
    {   // by default fill the next channel
        ch = next_ch_;
    }
    if (ch >= (int32_t)chans_)
    {
        return false;
    }

    const uint8_t *src = (const uint8_t *)buffer;
    uint8_t *dest = &(mem_[ch*bytes_per_sample_]);  // start of interleaved channel

    for (uint32_t s = 0; s < samples_per_block_; s++)
    {
        if (0u != dest[s*bytes_per_sample_*chans_])
        {
            cout << "sf_write_channel: warning: dest not zero!" << "\n";
        }
        if (8 == up_shift_bits_)
        {   // move one byte to the right (LITTLE ENDIAN!!!) in case of e.g. 24bit to 32 bit conversion
            memcpy(&(dest[s*bytes_per_sample_*chans_+1]), &(src[s*bytes_per_sample_]), bytes_per_sample_-1);
            dest[s*bytes_per_sample_*chans_] = 0;   // pad 8 LSBits (first byte for LE!)
        }
        else  // normal case
        {
            memcpy(&(dest[s*bytes_per_sample_*chans_]), &(src[s*bytes_per_sample_]), bytes_per_sample_);
        }
    }
    next_ch_ = ch + 1;  // prepare for next time

    return true;
}

bool WavFile::write()
{
    if (disable_writing_)
    {
        return true;  // don't do anything
    }

    if ((size_t)bytes_per_block_ != fwrite(mem_, (size_t)1, (size_t)bytes_per_block_, fp_))
    {
        return false;
    }

    memset(mem_, 0, bytes_per_block_);
    next_ch_ = 0;
    byte_pos_ += bytes_per_block_;

    return true;
}

WavFile::~WavFile()
{
    if (disable_writing_)
    {
        return;  // don't do anything
    }
    // update header with sizes
    wav_header.data_bytes = byte_pos_ - sizeof(Wav_header);
    wav_header.wav_size = byte_pos_ - 8;
    fseek(fp_, 0, SEEK_SET);
    fwrite(&wav_header, (size_t)1, sizeof(Wav_header), fp_);

    fclose(fp_);
}
