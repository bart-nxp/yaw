
#ifndef WAVFILE_HPP
#define WAVFILE_HPP

#include <stdio.h>
#include <stdint.h>


/*
    simple multichannel wav file writer
    integer PCM only
*/
class WavFile
{
private:
    bool disable_writing_ = false;
    uint32_t bytes_per_sample_ = 0u;
    uint32_t samples_per_block_ = 0u;
    uint32_t next_ch_ = 0u;
    unsigned char *mem_ = (unsigned char*) 0;                      // bytes_per_block bytes
    FILE *fp_ = (FILE *)0;
    uint32_t bytes_per_block_ = 0u;
    uint32_t chans_ = 0u;           // number of channels
    uint32_t up_shift_bits_ = 0u;  // set to 8 for 24 bit to 32 bit conversion
    uint32_t byte_pos_ = 0u;

    struct Wav_header {
        char riff_header[4] = {'R','I','F','F'};
        uint32_t wav_size = 0;                   // Size of the wav portion of the file. File size - 8
        char wave_header[4] = {'W','A','V','E'};
        char fmt_header[4] = {'f','m','t', ' '};
        uint32_t fmt_chunk_size = 16;            // 16 for PCM
        uint16_t audio_format = 1;               // 1 for PCM. 3 for IEEE Float
        uint16_t num_channels;
        uint32_t sample_rate;
        uint32_t byte_rate;                      // bytes per second. sample_rate * num_channels * Bytes Per Sample
        uint16_t sample_alignment;               // num_channels * Bytes Per Sample
        uint16_t bit_depth;                      //  bits per sample
        char data_header[4] = {'d','a','t','a'};
        uint32_t data_bytes;                     // Number of bytes in data. Num_samples * num_channels * sample byte size
    };
    struct Wav_header wav_header;

public:
    WavFile(uint32_t bytes_per_sample, uint32_t samples_per_block, unsigned char *mem, bool disable_writing);
    ~WavFile();

    bool open_write(const char *file_name, uint32_t sample_rate, uint32_t number_of_channels=1u);

    /*
        ch not set means: use channel 0 or the next channel from previous fill;
        next_ch will be reset to 0 when write() is called.
    */
    bool fill_channel(const void *buffer, int32_t ch=-1);

    bool write();
};

#endif
