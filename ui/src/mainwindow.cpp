#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QFile>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QAudioDeviceInfo>
#include <QSizePolicy>

#define PLOT_SAMPLES 2048

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Set desconstruction on close window
    this->setAttribute(Qt::WA_DeleteOnClose);
    QFont switch_btn_font(ui->actionLabel->font().family(), 10);
    int switch_option_pos = 1;

    m_controller = new Controller();

    m_ConvertSwitchButton = new Switch(this);
    m_ConvertSwitchButton->setChecked(false);
    m_ConvertSwitchButton->setLayoutDirection(Qt::RightToLeft);
    m_ConvertSwitchButton->setFont(switch_btn_font);
    m_ConvertSwitchButton->setText("Enable Conversion");
    ui->centerVLayout->insertWidget(switch_option_pos++, m_ConvertSwitchButton, 0, Qt::AlignLeft);

    m_PlaybackSwitchButton = new Switch(this);
    m_PlaybackSwitchButton->setChecked(true);
    m_PlaybackSwitchButton->setLayoutDirection(Qt::RightToLeft);
    m_PlaybackSwitchButton->setFont(switch_btn_font);
    m_PlaybackSwitchButton->setText("Enable Playback");
    ui->centerVLayout->insertWidget(switch_option_pos++, m_PlaybackSwitchButton, 0, Qt::AlignLeft);

    // Create two custom plot widgets for displaying the time domain and frequency domain plots
    // Create the time domain plot
    m_sourceAudioPlot = new QCustomPlot(this);
    m_sourceAudioPlot->addGraph();
    m_sourceAudioPlot->graph(0)->setPen(QPen(QColor(0,0,0)));
//    m_sourceAudioPlot->xAxis->setLabel("Time (s)");
    m_sourceAudioPlot->xAxis->setVisible(false);
//    m_sourceAudioPlot->yAxis->setLabel("Amplitude");
    m_sourceAudioPlot->yAxis->setVisible(false);
    m_sourceAudioPlot->xAxis->setRange(0, PLOT_SAMPLES);
    m_sourceAudioPlot->yAxis->setRange(-1, 1);
    m_sourceAudioPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_sourceAudioPlot->setInteraction(QCP::iRangeZoom, false);
    m_sourceAudioPlot->setInteraction(QCP::iRangeDrag, false);

    // Create the frequency domain plot
    m_outputAudioPlot = new QCustomPlot(this);
    m_outputAudioPlot->addGraph();
    m_outputAudioPlot->graph(0)->setPen(QPen(QColor(0,0,0)));
//    m_outputAudioPlot->xAxis->setLabel("Time (s)");
    m_outputAudioPlot->xAxis->setVisible(false);
//    m_outputAudioPlot->yAxis->setLabel("Amplitude");
    m_outputAudioPlot->yAxis->setVisible(false);
    m_outputAudioPlot->xAxis->setRange(0, PLOT_SAMPLES);
    m_outputAudioPlot->yAxis->setRange(-1, 1);
    m_outputAudioPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//    m_outputAudioPlot->setInteractions(QCP::iRangeZoom | QCP::iRangeDrag);
    m_outputAudioPlot->setInteraction(QCP::iRangeZoom, false);
    m_outputAudioPlot->setInteraction(QCP::iRangeDrag, false);

    for(int i = 0; i < PLOT_SAMPLES; ++i){
        m_sourceAudioDataKeys.push_back(i);
        m_sourceAudioDataValues.push_back(0);
        m_outputAudioDataKeys.push_back(i);
        m_outputAudioDataValues.push_back(0);
    }
    m_sourceAudioDataKeys.resize(PLOT_SAMPLES);
//    m_sourceAudioDataValues.resize(PLOT_SAMPLES);
    m_outputAudioDataKeys.resize(PLOT_SAMPLES);
//    m_outputAudioDataValues.resize(PLOT_SAMPLES);

    m_sourceAudioPlot->graph()->setData(m_sourceAudioDataKeys, m_sourceAudioDataValues);
    m_outputAudioPlot->graph()->setData(m_outputAudioDataKeys, m_outputAudioDataValues);

    // Add the custom plot widgets to the layout
    ui->plotsLayout->addWidget(m_sourceAudioPlot);
    ui->plotsLayout->addWidget(m_outputAudioPlot);

    // Init play button
    m_ActionLabel = ui->actionLabel;
    connect(m_ActionLabel, &LabelButton::clicked, this, &MainWindow::playButtonClicked);

    // add microphone to list
    QList<QAudioDeviceInfo> availableDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    for (const QAudioDeviceInfo &deviceInfo : availableDevices) {
        if(!deviceInfo.isFormatSupported(m_controller->input_audio_format))
            continue;
        QVariant deviceData;
        ui->inputsComboBox->addItem(deviceInfo.deviceName(), QVariant::fromValue<QAudioDeviceInfo>(deviceInfo));
    }

    availableDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    for (const QAudioDeviceInfo &deviceInfo : availableDevices) {
        if(!deviceInfo.isFormatSupported(m_controller->output_audio_format))
            continue;
        QVariant deviceData;
        ui->outputsComboBox->addItem(deviceInfo.deviceName(), QVariant::fromValue<QAudioDeviceInfo>(deviceInfo));
        ui->playbacksComboBox->addItem(deviceInfo.deviceName(), QVariant::fromValue<QAudioDeviceInfo>(deviceInfo));
    }
    ui->inputsComboBox->setSizeAdjustPolicy(ui->inputsComboBox->AdjustToContents);
    ui->inputsComboBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    ui->outputsComboBox->setSizeAdjustPolicy(ui->outputsComboBox->AdjustToContents);
    ui->outputsComboBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    ui->playbacksComboBox->setSizeAdjustPolicy(ui->playbacksComboBox->AdjustToContents);
    ui->playbacksComboBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
//    plot_timer = new QTimer(this);
//    connect(plot_timer, &QTimer::timeout, this, &MainWindow::updatePlots);
//    plot_timer->setInterval(50);
//    plot_timer->start();
}

MainWindow::~MainWindow()
{
//    if(m_controller != nullptr){
//        delete m_controller;
//        m_controller = nullptr;
//    }
////    delete ui;
////    delete m_sourceAudioPlot;
////    delete m_outputAudioPlot;
//    m_ConvertSwitchButton = nullptr;
//    m_ActionLabel = nullptr;
}

void MainWindow::modelInitProgress(float percentage)
{
    emit initProgress(percentage);
}

void MainWindow::updateSourcePlot(const std::vector<int16_t>& data)
{
    for(int i = 0; i < data.size(); ++i){
        m_sourceAudioDataValues.push_back((float)data[i] / 32768.0);
    }
    int cutoff = m_sourceAudioDataValues.size() - PLOT_SAMPLES;
    if(cutoff > 0)
        m_sourceAudioDataValues.erase(m_sourceAudioDataValues.begin(), m_sourceAudioDataValues.begin() + cutoff);
    m_sourceAudioPlot->graph()->setData(m_sourceAudioDataKeys, m_sourceAudioDataValues);
    m_sourceAudioPlot->replot(QCustomPlot::rpQueuedReplot);
}

void MainWindow::updateOutputPlot(const char* data, int nbytes)
{
    for(int i = 0; i < nbytes / 2; ++i){
        int16_t value = *(reinterpret_cast<const int16_t*>(data) + i);
        m_outputAudioDataValues.push_back((float)value / 32768.0);
    }
    int cutoff = m_outputAudioDataValues.size() - PLOT_SAMPLES;
    if(cutoff > 0)
        m_outputAudioDataValues.erase(m_outputAudioDataValues.begin(), m_outputAudioDataValues.begin() + cutoff);
    m_outputAudioPlot->graph()->setData(m_outputAudioDataKeys, m_outputAudioDataValues);
    m_outputAudioPlot->replot(QCustomPlot::rpQueuedReplot);
}

void MainWindow::initController()
{

    // Connect switch button to controller
    connect(m_ConvertSwitchButton, &Switch::clicked, m_controller, &Controller::setConvertEnable);
    connect(m_PlaybackSwitchButton, &Switch::clicked, m_controller, &Controller::setPlaybackEnable);

    connect(m_controller, &Controller::initProgress, this, &MainWindow::modelInitProgress);
    connect(m_controller, &Controller::sourceDataArrived, this, &MainWindow::updateSourcePlot);
    connect(m_controller, &Controller::outputDataArrived, this, &MainWindow::updateOutputPlot);
    m_controller->init(m_EncoderPath, m_DecoderPath);
    emit onInitialized();
}

bool MainWindow::checkFileExist(const QString filePath)
{
    QFile file(filePath);
    return file.exists();
}

void MainWindow::playButtonClicked()
{
    if (!m_isPlaying) {
        // Change button's text to Pause
        m_ActionLabel->setText("Pause");

        QVariant deviceData = ui->inputsComboBox->currentData();
        if (!deviceData.isNull()) {
            m_controller->setInput_device(deviceData.value<QAudioDeviceInfo>());
        }

        deviceData = ui->outputsComboBox->currentData();
        if (!deviceData.isNull()) {
            m_controller->setOutput_device(deviceData.value<QAudioDeviceInfo>());
        }

        deviceData = ui->playbacksComboBox->currentData();
        if (!deviceData.isNull()) {
            m_controller->setPlayback_device(deviceData.value<QAudioDeviceInfo>());
        }

        m_controller->start();

        m_isPlaying = true;
    } else {
        // Change button's text to Play
        m_ActionLabel->setText("Play");

        m_controller->stop();

        m_isPlaying = false;
    }
}

//void MainWindow::updatePlots(){
//    updateSourcePlot();
//    updateOutputPlot();
//}
