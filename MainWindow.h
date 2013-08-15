#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QTime>
#include "Streamer.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    void reset();
    ~MainWindow();

public slots:
    void onStartPressed(bool);
    void onConnectPressed(bool);
    void onModeEdited(QString);

    void onStreamError(Streamer::StreamError error);
    void onSocketError(QAbstractSocket::SocketError socketError, QString Error);
    void onSocketChanged(QAbstractSocket::SocketState socketState);
    void onStreamChanged(bool streaming);
    void drawCloud();
    
private:
    QTime time;
    unsigned timeouts;
    Ui::MainWindow *ui;
    QGraphicsScene* scene;
    Streamer stream;
};

#endif // MAINWINDOW_H
