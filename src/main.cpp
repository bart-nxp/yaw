#include "WavFile.hpp"
#include "assert.h"
#define _USE_MATH_DEFINES
#include "math.h"
#include <iostream>

#define SAMPLE_RATE 48000
#define SAMPLES_PER_FRAME 6400
#define NUM_CH 2

void generate_sine(int32_t* frame_buf, uint32_t num_samples, float amplitude_lin, float *state_ref)
{
    int i;
    float freq;
    float angle, increment, sample;
    float fullscale;

    angle = *state_ref;
    fullscale = (float)(1 << 23);  // max 24 bit LSB used
    freq = 2000.0;
    increment = 2 * M_PI * freq /(float)SAMPLE_RATE;
    for (i = 0; i < num_samples; i++)
    {
        sample = amplitude_lin * sinf(angle);
        frame_buf[i] = (int32_t)(floorf(sample*fullscale + 0.5));  // convert to 32 bit
        angle += increment;
    }
    if (angle >= 2 * M_PI)
    {
        angle -= 2 * M_PI;
    }
    *state_ref = angle;
}


std::string output_file_name = "out.wav";

int main(int argc, char * argv[])
{
    int32_t frames[NUM_CH][SAMPLES_PER_FRAME];

    std::cout << "Hello" << std::endl;

    float input_samples_states[NUM_CH] = {0.0};

    for (int ch = 0; ch < NUM_CH; ch++)
    {
        int32_t *ch_frame_in = frames[ch];
        generate_sine(ch_frame_in, SAMPLES_PER_FRAME, 0.05 * (ch + 1.0f), &(input_samples_states[ch]));
    }

    uint8_t sf_mem[sizeof(int32_t) * SAMPLES_PER_FRAME * NUM_CH]; // sound file mem
    assert(sizeof(sf_mem) >= NUM_CH * sizeof(frames[0]));

    WavFile sf(3, SAMPLES_PER_FRAME, sf_mem, false);
    assert(true == sf.open_write(output_file_name.c_str(), SAMPLE_RATE, NUM_CH));

    for (int ch = 0; ch < NUM_CH; ch++)
    {
        assert(true == sf.fill_channel(frames[ch]));
    }
    assert(true == sf.write());

    return 0;
}
