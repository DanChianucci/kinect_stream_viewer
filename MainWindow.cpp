#include <QtOpenGL>

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "errordialog.h"
#include "DebugUtil.h"

using std::string;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),ui(new Ui::MainWindow),stream()
{
    ui->setupUi(this);
    ui->canvas->giveDepthData(stream.getDepthData());
    ui->canvas->giveImageData(stream.getImageData());

    connect(&stream,SIGNAL(dataReady()),this,SLOT(drawCloud()));
    connect(&stream,SIGNAL(streamErrorOccurred(Streamer::StreamError)),this,SLOT(onStreamError(Streamer::StreamError)));
    connect(&stream,SIGNAL(streamChanged(bool)),this,SLOT(onStreamChanged(bool)));
    connect(&stream,SIGNAL(socketErrorOccurred(QAbstractSocket::SocketError,QString)),this,SLOT(onSocketError(QAbstractSocket::SocketError,QString)));
    connect(&stream,SIGNAL(socketChanged(QAbstractSocket::SocketState)),this,SLOT(onSocketChanged(QAbstractSocket::SocketState)));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete scene;
}

void MainWindow::reset()
{
    stream.reset();
}

void MainWindow::onStartPressed(bool state)
{
    dprintf("Start Pressed: %d\n",state);
    time.start();
    stream.setStreaming(state);
}

void MainWindow::onConnectPressed(bool state){
    dprintf("Connect Pressed: %d\n",state);
    stream.connectToSocket(ui->txtHostName->text(),ui->spinBox->value());
}

void MainWindow::drawCloud()
{
    int ms = time.restart();
    ui->FPS->setText(QString::number(1000.0/ms,'g',3));
    ui->canvas->update();
}

void MainWindow::onStreamError(Streamer::StreamError error)
{
    switch(error)
    {
    case Streamer::TimeOutError:
        timeouts++;
        ui->Timeouts->setText(QString::number(timeouts));
        break;

    case Streamer::NoDataError:
    {
        ErrorDialog *e = new ErrorDialog(string("The streamer was unable to collect from camera"));
        e->show();
        e->exec();
        delete e;

        reset();
        break;
    }

    case Streamer::NotConnectedError:
    {
        ErrorDialog * e = new ErrorDialog(string("Not Connected to streamer"));
        e->show();
        e->exec();
        delete e;
        reset();
        break;
    }

    case Streamer::InvalidData:
    {
        ErrorDialog *e = new ErrorDialog(string("The camera sent invalid data"));
        e->show();
        e->exec();
        delete e;
        reset();
        break;
    }

    default:
    {
        ErrorDialog * e = new ErrorDialog("An unknown streamer error occured");
        e->show();
        e->exec();
        delete e;
        reset();
        break;
    }
    }
}

void MainWindow::onSocketError(QAbstractSocket::SocketError socketError, QString Error)
{
    Error = QString("Socket Error: ")+QString::number(socketError)+QString("\n")+Error;
    ErrorDialog * e = new ErrorDialog(Error.toStdString());
    e->show();
    e->exec();
    delete e;
    reset();
}

void MainWindow::onSocketChanged(QAbstractSocket::SocketState socketState)
{
    bool checked =false;
    QString status("Socket is ");
    switch(socketState)
    {
    case QAbstractSocket::UnconnectedState:
        status+="unconnected.";
        ui->btnConnect->setChecked(false);
        break;
    case QAbstractSocket::HostLookupState:
        status+="looking up hostname...";

        break;
    case QAbstractSocket::ConnectingState:
        status+="connecting...";
        break;
    case QAbstractSocket::ConnectedState:
        status+="connected!";
        checked=true;
        break;
    case QAbstractSocket::BoundState:
        status+="bound.";
        break;
    case QAbstractSocket::ListeningState:
        status+="listening.";
        break;
    case QAbstractSocket::ClosingState:
        status+="closing";
        break;
    }
    ui->btnConnect->setChecked(checked);
    ui->statusBar->showMessage(status,10000);

}

void MainWindow::onStreamChanged(bool streaming)
{
    QString status = streaming ? "Stream started.":"Stream stopped.";
    ui->btnStartStream->setChecked(streaming);
    ui->statusBar->showMessage(status,1000);
}

void MainWindow::onModeEdited(QString mode)
{
    dprintf("Mode: %s\n",mode.toStdString().c_str());

    if(mode == "Depth")
    {
        stream.setMode(Streamer::STREAM_DEPTH);
        ui->canvas->setMode(CloudOpenGLPanel::PAINT_DEPTH);
    }


    else if(mode == "Disparity")
    {
        stream.setMode(Streamer::STREAM_DEPTH);
        ui->canvas->setMode(CloudOpenGLPanel::PAINT_DISPARITY);
    }

    else if(mode == "IR")
    {
        stream.setMode(Streamer::STREAM_IR);
        ui->canvas->setMode(CloudOpenGLPanel::PAINT_IR);
    }

    else if(mode == "RGB")
    {
        stream.setMode(Streamer::STREAM_RGB);
        ui->canvas->setMode(CloudOpenGLPanel::PAINT_RGB);
    }
    else if(mode == "Overlay")
    {
        stream.setMode(Streamer::STREAM_OVERLAY);
        ui->canvas->setMode(CloudOpenGLPanel::PAINT_OVERLAY);
    }
    else
    {
        stream.setStreaming(false);
        ui->canvas->setMode(CloudOpenGLPanel::PAINT_INVALID);
    }
}
