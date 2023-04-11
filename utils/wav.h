//
// Created by zyf on 4/9/23.
//

#ifndef VAD_SERVER_WAV_H
#define VAD_SERVER_WAV_H

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include "glog/logging.h"
#include <vector>

struct WaveHeader {
    char riff_tag[4];
    uint32_t riff_size;
    char wave_tag[4];
    char fmt_tag[4];
    uint32_t fmt_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char data_tag[4];
    uint32_t data_size;
};

WaveHeader ParseWaveHeader(const std::vector<char> &audio_data) {
    WaveHeader header;
    std::memcpy(&header, &audio_data[0], sizeof(header));
    return header;
}

std::vector<float> get_pcm_data(std::vector<char> &wave_data) {
    WaveHeader wave_header = ParseWaveHeader(wave_data);
    std::vector<char> pcm_data(wave_data.begin() + sizeof wave_header, wave_data.end());
    std::vector<float> float_pcm_data(wave_data.size() / sizeof(int16_t));
    const int16_t *pPcm = reinterpret_cast<const int16_t *>(pcm_data.data());
    std::transform(pPcm, pPcm + pcm_data.size() / sizeof(int16_t), float_pcm_data.begin(), [](int16_t s) { return s; });

    return float_pcm_data;
}


#endif //VAD_SERVER_WAV_H
