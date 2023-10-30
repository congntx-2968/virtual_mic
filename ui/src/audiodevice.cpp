#include "audiodevice.h"

AudioDevice::AudioDevice(QObject *parent) : QIODevice(parent)
{
    open(QIODevice::ReadWrite);
}

qint64 AudioDevice::readData(char *data, qint64 maxlen)
{
    QMutexLocker locker(&m_mutex);
    qint64 bytesToRead = qMin(qint64(m_buffer.size()), maxlen);
    if(bytesToRead > 0)
        memcpy(data, m_buffer.constData(), bytesToRead);
    else {
        memset(data, 0, maxlen);
        bytesToRead = maxlen;
    }

    m_buffer.remove(0, bytesToRead);
    locker.unlock();
    return bytesToRead;
}

qint64 AudioDevice::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data)
    Q_UNUSED(len)
    return 0;
}

void AudioDevice::onDataArrived(const char* data, int size)
{
//    QByteArray audioData;
//    for (int16_t sample : chunk) {
//        audioData.append(reinterpret_cast<const char*>(&sample), sizeof(sample));
//    }

    QMutexLocker locker(&m_mutex);
    m_buffer.append(data, size);
    locker.unlock();
}
