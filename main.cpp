
#include <QApplication>
#include <QTcpSocket>
#include <iostream>
#include <QThread>

#include "MainWindow.h"


using namespace std;

#define HOST_NAME "raspberrypi.local"
#define PORT 1234

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    return a.exec();
}
