#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>
#include <vector>
#include <cmath>

using std::cin;
using std::cout;
using std::endl;
using std::fstream;
using std::string;

#define MAX_WAV_VALUE 32767

typedef struct WAV_HEADER
{
    /* RIFF Chunk Descriptor */
    uint8_t RIFF[4] = {'R', 'I', 'F', 'F'};   // RIFF Header Magic header
    uint32_t chunkSize;                       // RIFF Chunk Size
    uint8_t format[4] = {'W', 'A', 'V', 'E'}; // format Header
    /* "fmt" sub-chunk */
    uint8_t subchunk1Id[4] = {'f', 'm', 't', ' '}; // FMT header
    uint32_t subchunk1Size = 16;                   // Size of the fmt chunk
    uint16_t audioFormat;                          // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
    uint16_t numChannels;                          // Number of channels 1=Mono 2=Sterio
    uint32_t sampleRate;                           // Sampling Frequency in Hz
    uint32_t byteRate;                             // bytes per second
    uint16_t blockAlign;                           // 2=16-bit mono, 4=16-bit stereo
    uint16_t bitsPerSample;                        // Number of bits per sample
    /* "data" sub-chunk */
    uint8_t subchunk2Id[4] = {'d', 'a', 't', 'a'}; // "data"  string
    uint32_t subchunk2Size;                        // Sampled data length
} wav_hdr;

/**
 * @brief Reads a WAV file and returns its audio data and sample rate.
 *
 * This function reads a WAV file from the given file path, extracts the
 * audio data as a vector of float values, and returns the audio data
 * along with the sample rate. The function supports only PCM audio format
 * and converts multi-channel audio to mono.
 *
 * @param path A string representing the file path of the WAV file to read.
 * @return A pair containing a vector of float values representing the audio data
 *         and a uint representing the sample rate of the audio data.
 * @throws std::runtime_error if the file cannot be opened or the audio format is unsupported.
 */
std::pair<std::vector<float>, unsigned int> __declspec(dllexport) read_wav(std::string path);

/**
 * @brief Saves audio data as a 16-bit little-endian WAV file.
 *
 * This function takes a vector of float values representing the audio samples,
 * a string representing the file path to save the audio file, and the audio
 * sample rate. It saves the audio data as a 16-bit little-endian WAV file with
 * the specified sample rate. The audio data is saved as a mono WAV file.
 *
 * @param audioData A vector of float values representing the input audio data.
 * @param path A string representing the file path to save the WAV file.
 * @param sampleRate A uint32_t representing the sample rate of the audio data.
 * @throws std::runtime_error if the file cannot be opened for writing.
 */
void __declspec(dllexport) save_wav(const std::string &path, const std::vector<float> &audioData, unsigned int sampleRate);


#endif // UTILS_H
