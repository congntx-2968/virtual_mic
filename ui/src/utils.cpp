#include "utils.h"

#include <samplerate.h>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <cmath>

std::vector<float> resampleAudio(const std::vector<float>& audioData, int originalSampleRate, int targetSampleRate) {
    if (originalSampleRate == targetSampleRate) {
        return audioData;
    }

    // Calculate the resampling ratio
    double ratio = static_cast<double>(targetSampleRate) / static_cast<double>(originalSampleRate);

    // Prepare the input and output arrays
    long inputFrames = static_cast<long>(audioData.size());
    long outputFrames = static_cast<long>(inputFrames * ratio) + 1;

    float* inputArray = const_cast<float*>(audioData.data());
    std::vector<float> outputData(static_cast<size_t>(outputFrames), 0.0f);
    float* outputArray = outputData.data();

    // Initialize the resampling converter
    int error;
    SRC_STATE* srcState = src_new(SRC_SINC_FASTEST, 1, &error);
    if (srcState == nullptr) {
        throw std::runtime_error("Failed to initialize libsamplerate: " + std::to_string(error));
    }

    // Set up the data structure for the resampling process
    SRC_DATA srcData;
    srcData.data_in = inputArray;
    srcData.input_frames = inputFrames;
    srcData.data_out = outputArray;
    srcData.output_frames = outputFrames;
    srcData.src_ratio = ratio;

    // Perform the resampling
    error = src_process(srcState, &srcData);
    if (error) {
        throw std::runtime_error("Error during resampling: " + std::to_string(error));
    }

    // Clean up and return the resampled audio data
    src_delete(srcState);
    outputData.resize(static_cast<size_t>(srcData.output_frames_gen));
    return outputData;
}

std::pair<std::vector<float>,  unsigned int> read_wav(std::string path)
{
    // Open the file in binary mode
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Failed to open file: " + path);
    }

    // Read the WAV file header
    wav_hdr header;

    file.read(reinterpret_cast<char *>(&header), sizeof(wav_hdr));
    if (
        std::string(reinterpret_cast<char *>(header.RIFF), 4) != "RIFF" ||
        std::string(reinterpret_cast<char *>(header.format), 4) != "WAVE" ||
        std::string(reinterpret_cast<char *>(header.subchunk1Id), 4) != "fmt ")
    {
        throw std::runtime_error("Wav file in wrong format!");
    }

    // Check if the audio format is supported
    if (header.audioFormat != 1)
    {
        throw std::runtime_error("Unsupported audio format: " + std::to_string(header.audioFormat));
    }

    // Read the audio data
    const uint32_t num_samples = header.subchunk2Size / (header.numChannels * (header.bitsPerSample / 8));
    std::vector<float> audio_data(num_samples);
    const float max_wav_value = static_cast<float>((uint64_t)1 << (header.bitsPerSample - 1));
    for (uint32_t i = 0; i < num_samples; i++)
    {
        float sample = 0.0f;
        for (uint16_t j = 0; j < header.numChannels; j++)
        {
            int16_t channel_sample;
            file.read(reinterpret_cast<char *>(&channel_sample), 2);
            sample += static_cast<float>(channel_sample) / max_wav_value;
        }
        audio_data[i] = sample / static_cast<float>(header.numChannels);
    }

    // Close the file
    file.close();

    // Return the audio data
    return {audio_data, header.sampleRate};
}

void save_wav(const std::string &path, const std::vector<float> &audioData, uint32_t sampleRate)
{
    // Prepare WAV header variables
    wav_hdr header;
    header.audioFormat = 1; // PCM
    header.numChannels = 1; // Mono audio
    header.sampleRate = sampleRate;
    header.bitsPerSample = 16;
    header.byteRate = header.sampleRate * header.numChannels * (header.bitsPerSample / 8);
    header.blockAlign = header.numChannels * (header.bitsPerSample / 8);
    header.subchunk2Size = static_cast<uint32_t>(audioData.size()) * header.numChannels * (header.bitsPerSample / 8);
    header.chunkSize = 36 + header.subchunk2Size;

    // Open the file for binary writing
    std::ofstream file(path, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Failed to open file: " + path);
    }

    // Write WAV header
    file.write(reinterpret_cast<const char *>(&header), sizeof(header));

    // Write audio data
    for (const auto &sample : audioData)
    {
        int16_t intSample = static_cast<int16_t>(std::round(sample * 32767));
        file.write(reinterpret_cast<const char *>(&intSample), 2);
    }

    // Close the file
    file.close();
}
