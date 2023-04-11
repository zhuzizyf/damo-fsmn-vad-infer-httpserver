//
// Created by zyf on 4/9/23.
//

#ifndef VAD_SERVER_FSMNVAD_H
#define VAD_SERVER_FSMNVAD_H

#include "e2e_vad.h"
#include "onnxruntime_cxx_api.h"
#include "kaldi-native-fbank/csrc/feature-fbank.h"
#include "kaldi-native-fbank/csrc/online-feature.h"


class FsmnVad {
public:
    FsmnVad();
    void test();
    void init_vad(const std::string &vad_model, int vad_sample_rate, int vad_silence_duration, int vad_max_len,
                  float vad_speech_noise_thres);

    std::vector<std::vector<int>> infer(const std::vector<float> &waves);

private:
    static void SplitStringToVectorFloat(const std::string &full, const char *delim,
                                         bool omit_empty_strings,
                                         std::vector<float> *out);

    void read_model(const std::string &vad_model);

    static void GetInputOutputInfo(
            const std::shared_ptr<Ort::Session> &session,
            std::vector<const char *> *in_names, std::vector<const char *> *out_names);

    void FbankKaldi(float sample_rate, std::vector<std::vector<float>> &vad_feats,
                    const std::vector<float> &waves);

    std::vector<std::vector<float>> &LfrCmvn(std::vector<std::vector<float>> &vad_feats, int lfr_m, int lfr_n);

    void Forward(
            const std::vector<std::vector<float>> &chunk_feats,
            std::vector<std::vector<float>> *out_prob);

    std::shared_ptr<Ort::Session> vad_session_ = nullptr;
    static Ort::Env env_;
    static Ort::SessionOptions session_options_;
    std::vector<const char *> vad_in_names_;
    std::vector<const char *> vad_out_names_;
    std::vector<std::vector<float>> cmvns;
    knf::FbankOptions fbank_opts;
    int vad_sample_rate_ = 16000;
    int vad_silence_duration_ = 800;
    int vad_max_len_ = 15000;
    double vad_speech_noise_thres_ = 0.9;
};


#endif //VAD_SERVER_FSMNVAD_H
