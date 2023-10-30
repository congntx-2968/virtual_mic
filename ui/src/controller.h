#ifndef CONTROLLER_H
#define CONTROLLER_H

#undef slots
#include "model.h"
#define slots Q_SLOTS

#include <QObject>
#include <QAudioInput>
#include <QAudioOutput>
#include <QtConcurrent/QtConcurrent>
#include <vector>
#include <QVector>
#include <cstdint>
#include <QByteArray>
#include <QMutex>

#include "audiodevice.h"
#include "audiosource.h"

class Controller : public QObject
{
    Q_OBJECT

public:
    explicit Controller(QObject *parent = nullptr);
    virtual ~Controller();

    void setInput_device(const QAudioDeviceInfo &device);
    void setOutput_device(const QAudioDeviceInfo &device);
    void setPlayback_device(const QAudioDeviceInfo &device);

    QAudioFormat input_audio_format;
    QAudioFormat output_audio_format;

private:
    void setup_audio_formats();
    QAudioDeviceInfo get_default_input_device();
    QAudioDeviceInfo get_default_output_device();
    QAudioDeviceInfo get_default_playback_device();
    void init_audio_devices();
    void init_stream_device();
    void init_playback_device();
    void init_model(const std::string& encoder_path, const std::string& decoder_path);
    void warmup_model();
    void start_streaming();
    void start_playback();
    void stop_streaming();
    void stop_playback();

signals:
    void initProgress(float percentage);
    void sourceDataArrived(const std::vector<int16_t>& data);
    void outputDataArrived(const char* data, int nbytes);

public slots:
    void init(const std::string& encoder_path, const std::string& decoder_path);
    void start();
    void stop();
    void onAudioReceived(const std::vector<int16_t>& audioData);
    std::vector<int16_t> process(const std::vector<int16_t>& audioData);
    void onProcessed();
    void setConvertEnable(bool enable);
    void setPlaybackEnable(bool enable);

private slots:
    void writeData(int length);

private:
    // Input and Ouput devices
    AudioSource *m_SourceDevice = nullptr;
    AudioDevice *m_DestDevice = nullptr;
    AudioDevice *m_PlaybackDevice = nullptr;
    QAudioInput *m_AudioInput = nullptr;
    QAudioOutput *m_AudioOutput = nullptr;
    QAudioOutput *m_AudioPlayback = nullptr;

    const std::string mic_name = "VCMic";
    QAudioDeviceInfo m_SourceDeviceInfo;
    QAudioDeviceInfo m_DestDeviceInfo;
    QAudioDeviceInfo m_PlaybackDeviceInfo;

    // Voice conversion model variables
    VITSWhisperV2 *model = nullptr;
    QFutureWatcher<std::vector<int16_t>> m_watcher;

    // buffer
    std::vector<int16_t> m_AudioBuffer;
    bool m_running = false;
    bool m_IsConvertEnable = false;
    bool m_IsPlayback = true;

    QByteArray m_buffer;
    QByteArray m_pbBuffer;
    QMutex m_mutex;
};

#endif // CONTROLLER_H
