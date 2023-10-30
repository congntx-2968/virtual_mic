#include "loadingscreen.h"
#include "ui_loadingscreen.h"

LoadingScreen::LoadingScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoadingScreen)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
}

LoadingScreen::~LoadingScreen()
{
}

void LoadingScreen::init()
{
    w = new MainWindow();
    QObject::connect(w, &MainWindow::initProgress, this, &LoadingScreen::initProgress);
    QObject::connect(w, &MainWindow::onInitialized, this, &LoadingScreen::onMainWindowInitialized);

    w->initController();
}

void LoadingScreen::initProgress(float percentage){
    float value = ui->progressBar->maximum() * percentage;
    ui->progressBar->setValue(static_cast<int>(value));
}

void LoadingScreen::onMainWindowInitialized(){
    this->hide();
    w->show();
}
