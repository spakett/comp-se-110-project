#include "dummyCore.h"
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    DummyCore core(w);
    w.setInputPort(&core);
    w.show();
    return a.exec();
}
