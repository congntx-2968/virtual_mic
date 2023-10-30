#include "model.h"
#include <cmath>

template <typename T>
torch::Dtype get_dtype()
{
    if (std::is_same<T, float>::value)
    {
        return torch::kFloat32;
    }
    else if (std::is_same<T, double>::value)
    {
        return torch::kFloat64;
    }
    else if (std::is_same<T, int8_t>::value)
    {
        return torch::kInt8;
    }
    else if (std::is_same<T, int16_t>::value)
    {
        return torch::kInt16;
    }
    else if (std::is_same<T, int32_t>::value)
    {
        return torch::kInt32;
    }
    else if (std::is_same<T, int64_t>::value)
    {
        return torch::kInt64;
    }
    else if (std::is_same<T, uint8_t>::value)
    {
        return torch::kUInt8;
    }
    else if (std::is_same<T, bool>::value)
    {
        return torch::kBool;
    }
    else
    {
        throw std::runtime_error("Unsupported type for Torch DType conversion.");
    }
}

template <typename T>
torch::Tensor to_tensor(const std::vector<T> &data, const std::vector<int64_t> &shape)
{
    // Convert the data to a torch::Tensor
    auto tensor_data = torch::tensor(data, get_dtype<T>());

    // Cast the tensor to the correct type
    return tensor_data.view(shape);
}

VITSWhisperV2::VITSWhisperV2(const std::string &encoder_path, const std::string &decoder_path)
{
    init_model(encoder_path, decoder_path);
}

VITSWhisperV2::VITSWhisperV2(std::istream &encoder_stream, std::istream &decoder_stream)
{
    init_model(encoder_stream, decoder_stream);
}

VITSWhisperV2::~VITSWhisperV2()
{
}

void VITSWhisperV2::init_model(const std::string &encoder_path, const std::string &decoder_path)
{
    std::ifstream encoder_stream(encoder_path, std::ios_base::binary | std::ios_base::in);
    std::ifstream decoder_stream(decoder_path, std::ios_base::binary | std::ios_base::in);
    if (!encoder_stream.is_open())
    {
        std::stringstream ss;
        ss << "Cannot load encoder from: " << encoder_path;
        throw std::runtime_error(ss.str());
    }
    if (!decoder_stream.is_open())
    {
        std::stringstream ss;
        ss << "Cannot load decoder from: " << decoder_path;
        throw std::runtime_error(ss.str());
    }
    init_model(encoder_stream, decoder_stream);
    encoder_stream.close();
    decoder_stream.close();
}

void VITSWhisperV2::init_model(std::istream &encoder_stream, std::istream &decoder_stream)
{
    try
    {
        // Load the TorchScript model from file
        this->encoder = torch::jit::load(encoder_stream);
        this->encoder.eval();
        this->decoder = torch::jit::load(decoder_stream);
        this->decoder.eval();

        vad_options default_option;
        vad.set_options(default_option);
        vad.init_model();
    }
    catch (const c10::Error &e)
    {
        std::stringstream ss;
        ss << "Error loading the model: " << e.what();
        throw std::runtime_error(ss.str());
        return;
    }
}

std::vector<float> VITSWhisperV2::inference(const std::vector<float> &x)
{
    if (x.size() != SEGMENT_SIZE)
    {
        std::stringstream ss;
        ss << "Invalid input size. Expect " << SEGMENT_SIZE << " but got " << x.size();
        throw std::runtime_error(ss.str());
    }
    torch::Tensor input = to_tensor(x, {SEGMENT_SIZE});

    return inference(input);
}

std::vector<float> VITSWhisperV2::inference(const std::vector<int16_t> &x)
{
    if (x.size() != SEGMENT_SIZE)
    {
        std::stringstream ss;
        ss << "Invalid input size. Expect " << SEGMENT_SIZE << " but got " << x.size();
        throw std::runtime_error(ss.str());
    }
    // convert to float tensor and normalize to [-1, 1]
    torch::Tensor input = to_tensor(x, {SEGMENT_SIZE}).to(get_dtype<float>());
    input /= 32767.0f;
    return inference(input);
}

std::vector<float> VITSWhisperV2::inference(const torch::Tensor &x)
{
    if (x.ndimension() != 1)
    {
        std::stringstream ss;
        ss << "Invalid input dimensions. Expect " << 1 << " but got " << x.ndimension();
        throw std::runtime_error(ss.str());
    }
    if (x.size(0) != SEGMENT_SIZE)
    {
        std::stringstream ss;
        ss << "Invalid input size. Expect " << SEGMENT_SIZE << " but got " << x.size(0);
        throw std::runtime_error(ss.str());
    }

    // Pass the input tensors through the model
    torch::Tensor phone = encoder.forward({x}).toTensor();
    torch::Tensor y = decoder.forward({phone.cuda()}).toTensor().cpu().contiguous();

    std::vector<int> states = vad.get_speech_states(x);
    auto states_tensor = to_tensor(states, {(long)states.size()});

    auto output = y * states_tensor;

    // // Return the output tensor
    std::vector<float> v(output.data_ptr<float>(), output.data_ptr<float>() + output.numel());
    return v;
}
