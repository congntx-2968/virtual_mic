#ifndef LOADINGSCREEN_H
#define LOADINGSCREEN_H

#include <QWidget>
#include "mainwindow.h"

namespace Ui {
class LoadingScreen;
}

class LoadingScreen : public QWidget
{
    Q_OBJECT

public:
    explicit LoadingScreen(QWidget *parent = nullptr);
    ~LoadingScreen();
public slots:
    void initProgress(float percentage);
    void onMainWindowInitialized();
    void init();

private:
    Ui::LoadingScreen *ui;
    MainWindow *w;
};

#endif // LOADINGSCREEN_H
