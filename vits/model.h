#ifndef MODEL_H
#define MODEL_H

#include <torch/torch.h>
#include <torch/script.h>

#include <vector>
#include <algorithm>
#include <ctime>

#include "vad.h"

#define SEGMENT_SIZE 16384 // Size of one audio segment

template <typename T>
torch::Dtype get_dtype();

template <typename T>
torch::Tensor to_tensor(const std::vector<T> &data, const std::vector<int64_t> &shape);

class __declspec(dllexport) VITSWhisperV2{
public:
    explicit VITSWhisperV2(const std::string &encoder_path, const std::string &decoder_path);
    explicit VITSWhisperV2(std::istream &encoder_stream, std::istream &decoder_stream);
    virtual ~VITSWhisperV2();

    std::vector<float> inference(const std::vector<float>& x);
    std::vector<float> inference(const std::vector<int16_t>& x);
    std::vector<float> inference(const torch::Tensor& x);

private:
    torch::jit::Module encoder;
    torch::jit::Module decoder;
    void init_model(const std::string &encoder_path, const std::string &decoder_path);
    void init_model(std::istream &encoder_stream, std::istream &decoder_stream);

    VAD vad;
};
#endif //MODEL_H
