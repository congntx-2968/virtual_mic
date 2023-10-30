#include "controller.h"
#include <algorithm>
#include "utils.h"

#include <chrono>

unsigned long long time_ns()
{
    using namespace std::chrono;
    return duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
}

Controller::Controller(QObject *parent) : QObject(parent)
{
    m_IsConvertEnable = false;
    m_IsPlayback = true;
    setup_audio_formats();
    m_SourceDeviceInfo = get_default_input_device();
    m_DestDeviceInfo = get_default_output_device();
    m_PlaybackDeviceInfo = get_default_playback_device();
}

Controller::~Controller(){
    if(m_running)
        stop();
    if(m_SourceDevice != nullptr){
        delete m_SourceDevice;
        m_SourceDevice = nullptr;
    }
    if(m_DestDevice != nullptr){
        delete m_DestDevice;
        m_DestDevice = nullptr;
    }
    if(m_AudioInput != nullptr){
        delete m_AudioInput;
        m_AudioInput = nullptr;
    }
    if(m_AudioOutput != nullptr){
        delete m_AudioOutput;
        m_AudioOutput = nullptr;
    }
}

void Controller::setInput_device(const QAudioDeviceInfo &device)
{
    m_SourceDeviceInfo = device;
}

void Controller::setOutput_device(const QAudioDeviceInfo &device)
{
    m_DestDeviceInfo = device;
}

void Controller::setPlayback_device(const QAudioDeviceInfo &device)
{
    m_PlaybackDeviceInfo = device;
}

void Controller::setup_audio_formats() {
    // Input: mono 16bit/pcm - 16000Hz
    input_audio_format.setSampleRate(16000);
    input_audio_format.setChannelCount(1);
    input_audio_format.setSampleSize(16);
    input_audio_format.setCodec("audio/pcm");
    input_audio_format.setByteOrder(QAudioFormat::LittleEndian);
    input_audio_format.setSampleType(QAudioFormat::SignedInt);

    // Output: mono 16bit/pcm - 24000Hz
    output_audio_format.setSampleRate(24000);
    output_audio_format.setChannelCount(1);
    output_audio_format.setSampleSize(16);
    output_audio_format.setCodec("audio/pcm");
    output_audio_format.setByteOrder(QAudioFormat::LittleEndian);
    output_audio_format.setSampleType(QAudioFormat::SignedInt);

}

QAudioDeviceInfo Controller::get_default_input_device() {
    QList<QAudioDeviceInfo> availableDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    QAudioDeviceInfo device;
    for (const QAudioDeviceInfo &deviceInfo : availableDevices) {
        std::cout << deviceInfo.deviceName().toStdString() << std::endl;
        if(deviceInfo.deviceName().startsWith("Microphone")){
            if(deviceInfo.isFormatSupported(input_audio_format)){
                device = deviceInfo;
                break;
            }
        }
    }
    return device;
}

QAudioDeviceInfo Controller::get_default_output_device() {
    QList<QAudioDeviceInfo> availableDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    QAudioDeviceInfo device;
    for (const QAudioDeviceInfo &deviceInfo : availableDevices) {
        std::cout << deviceInfo.deviceName().toStdString() << std::endl;
        if(deviceInfo.deviceName().startsWith("CABLE Input")){
            if(deviceInfo.isFormatSupported(output_audio_format)){
                device = deviceInfo;
                break;
            }
        }
    }
    return device;
}

QAudioDeviceInfo Controller::get_default_playback_device() {
    QList<QAudioDeviceInfo> availableDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    QAudioDeviceInfo device;
    for (const QAudioDeviceInfo &deviceInfo : availableDevices) {
        std::cout << deviceInfo.deviceName().toStdString() << std::endl;
        if(deviceInfo.deviceName().startsWith("Speakers")){
            if(deviceInfo.isFormatSupported(output_audio_format)){
                device = deviceInfo;
                break;
            }
        }
    }
    return device;
}

void Controller::init_audio_devices(){
    init_stream_device();
    init_playback_device();
}

void Controller::init_stream_device(){
    if(m_SourceDevice != nullptr)
        delete m_SourceDevice;
    m_SourceDevice = new AudioSource(this);
    // connect input device with callback
    connect(m_SourceDevice, &AudioSource::audioReceived, this, &Controller::onAudioReceived);
}

void Controller::init_playback_device(){
    if(m_DestDevice != nullptr)
        delete m_DestDevice;
    m_DestDevice = new AudioDevice(this);

    if(m_PlaybackDevice != nullptr)
        delete m_PlaybackDevice;
    m_PlaybackDevice = new AudioDevice(this);
}

void Controller::init_model(const std::string& encoder_path, const std::string& decoder_path){
    if(model != nullptr)
        delete model;
    model = new VITSWhisperV2(encoder_path, decoder_path);
    warmup_model();
}

void Controller::warmup_model(){
    qDebug() << "Warm up model 10 times: ";
    uint64_t beg;
    for (int i = 1; i <= 10; ++i){
        beg = time_ns();

        torch::Tensor x = torch::rand(SEGMENT_SIZE, at::kFloat) * 2 - 1;
        std::vector<float> output = model->inference(x);

        float ns = time_ns() - beg;
        qDebug() << i << ": " << ns / 1e6 << "ms";
        std::cout << i << ": " << ns / 1e6 << "ms" << std::endl;
        emit initProgress(0.1 * i);
    }
}

void Controller::start_streaming()
{
    if(m_SourceDeviceInfo.isNull())
        throw std::runtime_error("Microphone is not found");
    else
        std::cout << "Input: " << m_SourceDeviceInfo.deviceName().toStdString() << std::endl;

    if(m_DestDeviceInfo.isNull())
        throw std::runtime_error("Destination endpoint is not found");
    else
        std::cout << "Output: " << m_DestDeviceInfo.deviceName().toStdString() << std::endl;

    // Release audio input if exist
    if(m_AudioInput != nullptr)
        delete m_AudioInput;

    m_AudioInput = new QAudioInput(m_SourceDeviceInfo, input_audio_format, this);
    if(m_AudioInput != nullptr && m_SourceDevice != nullptr)
        m_AudioInput->start(m_SourceDevice);


    // Release audio output if exist
    if(m_AudioOutput != nullptr)
        delete m_AudioOutput;

    m_AudioOutput = new QAudioOutput(m_DestDeviceInfo, output_audio_format, this);
    if(m_AudioOutput != nullptr && m_DestDevice != nullptr)
        m_AudioOutput->start(m_DestDevice);
}

void Controller::start_playback()
{
    if(m_PlaybackDeviceInfo.isNull()) {
        qWarning() << "Playback device is not found. Abort playback!";
        setPlaybackEnable(false);
    }    else
        std::cout << "Playback: " << m_PlaybackDeviceInfo.deviceName().toStdString() << std::endl;

    // Release audio input if exist
    if(m_AudioPlayback != nullptr)
        delete m_AudioPlayback;

    m_AudioPlayback = new QAudioOutput(m_PlaybackDeviceInfo, output_audio_format, this);
    if(m_AudioPlayback != nullptr && m_PlaybackDevice != nullptr)
        m_AudioPlayback->start(m_PlaybackDevice);
}

void Controller::stop_streaming()
{
    if(m_AudioInput != nullptr){
        m_AudioInput->stop();
        delete m_AudioInput;
        m_AudioInput = nullptr;
    }

    if(m_AudioOutput != nullptr){
        m_AudioOutput->stop();
        delete m_AudioOutput;
        m_AudioOutput = nullptr;
    }
}

void Controller::stop_playback()
{
    if(m_AudioPlayback != nullptr){
        m_AudioPlayback->stop();
        delete m_AudioPlayback;
        m_AudioPlayback = nullptr;
    }
}

void Controller::init(const std::string& encoder_path, const std::string& decoder_path){
    init_audio_devices();
    init_model(encoder_path, decoder_path);
    // callback for asynchronous processing
    connect(&m_watcher, &QFutureWatcher<std::vector<int16_t>>::finished, this, &Controller::onProcessed);
}

void Controller::start(){
    m_running = true;
    start_streaming();
    start_playback();
}

void Controller::stop(){
    m_running = false;

    m_AudioBuffer.clear();
    m_watcher.cancel();

    QMutexLocker lock(&m_mutex);
    m_buffer.clear();
    m_pbBuffer.clear();
    lock.unlock();
    stop_playback();
    stop_streaming();
}

void Controller::onAudioReceived(const std::vector<int16_t>& audioData){
    QFuture<void> async_write = QtConcurrent::run(this, &Controller::writeData, audioData.size() * 3);
    emit sourceDataArrived(audioData);

    m_AudioBuffer.insert(m_AudioBuffer.end(), audioData.cbegin(), audioData.cend());

    if(m_AudioBuffer.size() >  2 * SEGMENT_SIZE)
        qWarning() << "Buffer size is increase to large!!!";

    if(m_AudioBuffer.size() > SEGMENT_SIZE){
        std::vector<int16_t> data(SEGMENT_SIZE, 0);
        std::copy(m_AudioBuffer.cbegin(), m_AudioBuffer.cbegin() + SEGMENT_SIZE, data.begin());
        m_AudioBuffer.erase(m_AudioBuffer.cbegin(), m_AudioBuffer.cbegin() + SEGMENT_SIZE);
        QFuture<std::vector<int16_t>> future = QtConcurrent::run(this, &Controller::process, data);
        m_watcher.setFuture(future);
    }
}

std::vector<int16_t> Controller::process(const std::vector<int16_t>& audioData){

    std::vector<float> data(SEGMENT_SIZE, 0);
    std::vector<float> output;
    size_t size = SEGMENT_SIZE > audioData.size() ? audioData.size() : SEGMENT_SIZE;

    for(size_t i = 0; i < size; ++i){
        data[i] = static_cast<float>(audioData[i]) / 32767;
    }

    if(!m_IsConvertEnable){
        output = resampleAudio(data, 16000, 24000);
    }else{
        uint64_t beg = time_ns();

        output = model->inference(data);

        float ns = time_ns() - beg;
        qDebug() << "Inference time: " << ns / 1e6 << "ms";
    }

    std::vector<int16_t> result(output.size(), 0);
    size_t output_size = output.size();

    for(size_t i = 0; i < output_size; ++i){
        result[i] = static_cast<int16_t>(output[i] * 32767);
    }
    return result;
}

void Controller::onProcessed(){
    if(m_running && !m_watcher.isCanceled()){
        std::vector<int16_t> audioData = m_watcher.result();
        if(m_IsPlayback)
            m_PlaybackDevice->onDataArrived((char*)audioData.data(), audioData.size() * sizeof(int16_t));
        m_DestDevice->onDataArrived((char*)audioData.data(), audioData.size() * sizeof(int16_t));

        QMutexLocker locker(&m_mutex);
        m_buffer.append((char*)audioData.data(), audioData.size() * sizeof(int16_t));
        locker.unlock();
    }
}

void Controller::setConvertEnable(bool enable)
{
    m_IsConvertEnable = enable;
}

void Controller::setPlaybackEnable(bool enable)
{
    m_IsPlayback = enable;
}

void Controller::writeData(int length)
{
    length += length % 2;
    QMutexLocker locker(&m_mutex);
    if(!m_buffer.isEmpty()){
        size_t nbytes = std::min(m_buffer.size(), length);
        emit outputDataArrived(m_buffer.data(), nbytes);
        m_buffer.remove(0, nbytes);
    }

    locker.unlock();
}
