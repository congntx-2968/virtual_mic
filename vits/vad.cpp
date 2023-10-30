#include "vad.h"
#include "silero_vad.h"
#include <iostream>
#include <algorithm>

VAD::VAD()
{
}

VAD::VAD(const vad_options &options)
{
    this->set_options(options);
}

void VAD::set_options(const vad_options &options)
{
    this->options = options;
    if (options.sampling_rate > 16000 && (options.sampling_rate % 16000 == 0))
    {
        step = options.sampling_rate / 16000;
        this->options.sampling_rate = 16000;

        std::cerr << "Sampling rate is a multiple of 16000, casting to 16000 manually!\n";
    }
    else
    {
        step = 1;
    }

    if (options.sampling_rate == 8000 && options.chunk_size > 768)
    {
        std::cerr << "chunk_size is too big for 8000 sampling_rate! Better set chunk_size to 256, 512, or 768 for 8000 sample rate!";
    }

    std::vector<int> supported_chunk_sizes_16000 = {512, 1024, 1536};
    std::vector<int> supported_chunk_sizes_8000 = {256, 512, 768};

    bool is_supported = (options.sampling_rate == 16000 && std::find(supported_chunk_sizes_16000.begin(), supported_chunk_sizes_16000.end(), (int)options.chunk_size) != supported_chunk_sizes_16000.end()) ||
                        (options.sampling_rate == 8000 && std::find(supported_chunk_sizes_8000.begin(), supported_chunk_sizes_8000.end(), (int)options.chunk_size) != supported_chunk_sizes_8000.end());

    if (!is_supported)
    {
        std::cerr << "Unusual chunk_size! Supported chunk_size:"
                  << "\n - [512, 1024, 1536] for 16000 sampling_rate"
                  << "\n - [256, 512, 768] for 8000 sampling_rate";
    }
}

void VAD::init_model()
{
    SileroVAD::init_model();
}

std::vector<int> VAD::get_speech_states(const std::vector<float> &speech)
{
    torch::Tensor speech_tensor = torch::tensor(speech, torch::kFloat);
    return get_speech_states(speech_tensor);
}

std::vector<int> VAD::get_speech_states(const torch::Tensor &speech)
{
    std::vector<float> speech_probs;
    int audio_length_samples = speech.size(0);
    int target_length_samples = audio_length_samples * options.target_sampling_rate / options.sampling_rate;
    std::vector<int> speech_states;

    for (int start = 0, end = start + options.chunk_size; start < audio_length_samples; start += options.chunk_size, end = std::min(start + options.chunk_size, audio_length_samples))
    {
        auto seg = speech.slice(0, start, end);
        if (seg.size(0) < options.chunk_size)
            seg = torch::pad(seg, {0, options.chunk_size - seg.size(0)}, "constant", 0);
        float prob = SileroVAD::predict(seg, options.sampling_rate);
        speech_probs.push_back(prob);
    }

    bool triggered = prev_state == ACTIVATED;
    int cur_start = 0;
    int cur_end = 0;
    std::vector<std::pair<int, int>> speech_segments;
    int temp_end = 0; // to save potential segment end (and tolerate some silence)

    for (size_t i = 0; i < speech_probs.size(); ++i)
    {
        float speech_prob = speech_probs[i];
        // if has silence frame but only in short time
        if (speech_prob >= options.threshold && temp_end)
        {
            temp_end = 0;
        }

        // if start of speech
        if (speech_prob >= options.threshold && !triggered)
        {
            triggered = true;
            cur_start = i;
            continue;
        }

        // if start of silence
        if (speech_prob < options.neg_threshold && triggered)
        {
            // set temp_end to check if silence long or short
            if (!temp_end)
                temp_end = i;
            // if short
            if (i - temp_end < options.min_silence_duration_frames)
                continue;
            else
            {
                cur_end = temp_end;
                if (cur_end - cur_start > options.min_speech_duration_frames)
                    speech_segments.push_back({cur_start, cur_end});
                cur_start = cur_end = temp_end = 0;
                triggered = false;
            }
        }
    }

    if ((cur_start || triggered) && !cur_end)
    {
        cur_end = speech_probs.size() - 1;
        speech_segments.push_back({cur_start, cur_end});
    }

    int prev_end = 0;
    int target_chunk_size = options.chunk_size * options.target_sampling_rate / options.sampling_rate;
    if (!speech_segments.empty())
    {
        for (auto [s, e] : speech_segments)
        {
            speech_states.insert(speech_states.end(), (s - prev_end) * target_chunk_size, 0);
            speech_states.insert(speech_states.end(), (e - s + 1) * target_chunk_size, 1);
            prev_end = e;
            // std::cout << "Start: " << s << " , stop: " << e << std::endl;
        }
    }
    else
    {
        speech_states.assign(target_length_samples, 0);
    }
    if (speech_states.size() < target_length_samples)
        speech_states.insert(speech_states.end(), target_length_samples - speech_states.size(), 0);
    else if (speech_states.size() > target_length_samples)
        speech_states.erase(speech_states.begin() + target_length_samples, speech_states.end());

    prev_state = triggered ? ACTIVATED : INACTIVATED;

    return speech_states;
}
