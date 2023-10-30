#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QtMultimedia/QAudioOutput>
#include <QBuffer>
#include <QVector>
#include <QMutex>
#include <QTimer>

#include "qcustomplot.h"
#include "controller.h"
#include "switch.h"
#include "labelbutton.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private:
    Ui::MainWindow *ui;

signals:
    void initProgress(float percentage);
    void onInitialized();

public slots:
    void initController();
    void modelInitProgress(float percentage);
//    void onSourceDataArrived(const std::vector<int16_t>& data);
//    void onOutputDataArrived(const char* data, int nbytes);

private slots:
    void updateSourcePlot(const std::vector<int16_t>& data);
    void updateOutputPlot(const char* data, int nbytes);
    void playButtonClicked();
//    void updatePlots();

private:
    std::string m_EncoderPath = "weights\\encoder.pt";
    std::string m_DecoderPath = "weights\\decoder.pt";
    QCustomPlot *m_sourceAudioPlot = nullptr;
    QCustomPlot *m_outputAudioPlot = nullptr;
    QVector<double> m_sourceAudioDataKeys;
    QVector<double> m_sourceAudioDataValues;
    QVector<double> m_outputAudioDataKeys;
    QVector<double> m_outputAudioDataValues;

    Switch *m_DenoiseSwitchButton = nullptr;
    Switch *m_ConvertSwitchButton = nullptr;
    Switch *m_PlaybackSwitchButton = nullptr;
    LabelButton *m_ActionLabel = nullptr;

    bool m_isPlaying;

    Controller *m_controller = nullptr;

    bool checkFileExist(const QString filePath);
//    QMutex source_plot_mutex;
//    QMutex output_plot_mutex;
//    std::vector<int16_t> sourceDataBuffer;
//    std::vector<int16_t> outputDataBuffer;

//    QTimer *plot_timer;
};
#endif // MAINWINDOW_H
