#include "loadingscreen.h"
#include <torch/csrc/jit/passes/tensorexpr_fuser.h>
#include <QApplication>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    torch::AutoGradMode enable_grad(false);
    torch::jit::setTensorExprFuserEnabled(false);

    LoadingScreen l;
    l.show();
    QTimer::singleShot(1000, &l, &LoadingScreen::init);
    int ret;
    try{
        ret = a.exec();
    }catch(std::exception e){
        std::cerr << e.what() << std::endl;
    }
    return ret;
}
