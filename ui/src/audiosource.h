#ifndef AUDIOSOURCE_H
#define AUDIOSOURCE_H

#include <QIODevice>

class AudioSource : public QIODevice
{
    Q_OBJECT
public:
    explicit AudioSource(QObject *parent = nullptr);

    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

signals:
    void audioReceived(const std::vector<int16_t>& audioData);
};

#endif // AUDIOSOURCE_H
