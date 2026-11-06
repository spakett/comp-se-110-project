#include "QtNetworkClient.h"
#include "apiHandler.h"
#include "core.h"
#include "mainwindow.h"
#include "sqliteDatabase.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    QtNetworkClient networkClient;
    SqliteDatabase database("air-quality-cache.sqlite");
    ApiHandler apiHandler(networkClient, database);
    Core core(w, apiHandler);
    apiHandler.setCallback(core);
    networkClient.setCallback(apiHandler);
    w.setInputPort(&core);
    w.show();
    return a.exec();
}