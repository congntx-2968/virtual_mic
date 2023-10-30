#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

#include <QIODevice>
#include <QByteArray>
#include <QMutex>
#include <vector>
#include <cstdint>

#define BUFFER_LOW_THRESHOLD 16384  // Adjust this value according to your needs

class AudioDevice : public QIODevice
{
    Q_OBJECT

public:
    explicit AudioDevice(QObject *parent = nullptr);

    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

public slots:
    void onDataArrived(const char* data, int size);

private:
    QByteArray m_buffer;
    QMutex m_mutex;
};

#endif // AUDIODEVICE_H
