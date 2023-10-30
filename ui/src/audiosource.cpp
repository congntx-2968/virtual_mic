#include "audiosource.h"
#include <vector>

AudioSource::AudioSource(QObject *parent) : QIODevice(parent)
{
    open(QIODevice::ReadWrite);
}

qint64 AudioSource::readData(char *data, qint64 maxlen)
{
    Q_UNUSED(data);
    Q_UNUSED(maxlen);
    return 0;
}

qint64 AudioSource::writeData(const char* data, qint64 len){
    const int16_t* samples = reinterpret_cast<const int16_t*>(data);
    int numSamples = len / sizeof(int16_t);
    std::vector<int16_t> audioData(numSamples);
    std::copy(samples, samples + numSamples, audioData.begin());

    emit audioReceived(audioData);

    return len;
}
