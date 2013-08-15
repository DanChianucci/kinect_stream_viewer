#include "Streamer.h"
#include "DebugUtil.h"
#include <string>
Streamer::Streamer(QObject *parent) :
    QObject(parent)
{
    memset(depthBuf.data,0,BUF_SIZE);
    memset(imageBuf.data,0,BUF_SIZE);
    mode = STREAM_DEPTH;

    //Connect the Socket Signals
    connect(this,SIGNAL(dataReady()),this,SLOT(updateData()));
    connect(&socket,SIGNAL(readyRead()),this,SLOT(socketRecievedData()));
    connect(&socket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(socketError(QAbstractSocket::SocketError)));
    connect(&socket,SIGNAL(stateChanged(QAbstractSocket::SocketState)),this,SLOT(socketStateChanged(QAbstractSocket::SocketState)));
}

//******************************************************************************
// Methods
//******************************************************************************
void Streamer::connectToSocket(const QString& host, int port)
{
    if(socket.state()==QAbstractSocket::UnconnectedState)
    {
        dprintf("Connecting to Socket \"%s\"   %d\n",host.toStdString().c_str(),port);
        socket.connectToHost(host,port);
    }
    else
    {
        dprintf("Socket Already In Use\n");
        socket.close();
    }
}

void Streamer::setStreaming(bool set)
{
    bool prev = running;
    if(socket.state()==QAbstractSocket::ConnectedState && set)
    {
        running = true;
        this->updateData();
    }

    else
        running=false;

    if(running!=prev)
    {
        dprintf("Streamer: %d\n",running);
        emit(streamChanged(running));
    }
}

void Streamer::setMode(STREAMMODE m)
{
    dprintf("Mode Set to: %d\n",m);
    mode = m;
}

void Streamer::reset()
{
    setStreaming(false);
    memset(depthBuf.data,0,BUF_SIZE);
    memset(imageBuf.data,0,BUF_SIZE);
}

//******************************************************************************
// Slots
//******************************************************************************
void Streamer::updateData()
{
    if(running)
    {
        buffFull=false;
        buffNum=1;
        index=0;

        QString getCommand;
        switch( mode)
        {
        case STREAM_DEPTH:
            getCommand = "getDepth"; break;
        case STREAM_IR:
            getCommand = "getIR"; break;
        case STREAM_RGB:
            getCommand = "getRGB"; break;
        case STREAM_OVERLAY:
            getCommand = "getBoth"; break;
        }
        socket.write(getCommand.toStdString().c_str(),getCommand.length());

    }

}

void Streamer::socketRecievedData()
{


    if(mode == STREAM_OVERLAY)
    {
        buffer_t &tmp = (buffNum==1) ? depthBuf : imageBuf;
        fillBuffer(tmp);

        if(buffFull)
        {
            if(buffNum==1){
                index=0;
                buffNum=2;
            }
            else
               emit(dataReady());
        }
    }

    else  //depth, IR, RGB
    {
        buffer_t &tmp = (mode==STREAM_DEPTH) ? depthBuf : imageBuf;
        fillBuffer(tmp);

        if(buffFull)
            emit(dataReady());
    }
}

void Streamer::fillBuffer(buffer_t &buf)
{
    qint64 read = socket.read(buf.data+index,BUF_SIZE-index);

    if(read<0)
    {
        emit(streamErrorOccurred(NotConnectedError));
    }
    else if( strcmp( buf.data,"FAILED")==0)
    {
        emit(streamErrorOccurred(NoDataError));
    }
    else if(strcmp(buf.data,"INVALID")==0)
    {
        emit(streamErrorOccurred(InvalidData));
    }
    else
    {
        index+=read;
        if(index==BUF_SIZE)
        {
            buffFull=true;
        }
    }
}

void Streamer::socketError(QAbstractSocket::SocketError socketError)
{
    dprintf("Socket Error %d: %s\n",socket.error(),socket.errorString().toStdString().c_str());
    emit(socketErrorOccurred(socketError,socket.errorString()));
    socket.close();
    setStreaming(false);
}

void Streamer::socketStateChanged(QAbstractSocket::SocketState socketState)
{
    dprintf("Socket State Changed: %d\n",socket.state());
    emit(socketChanged(socketState));

    switch(socketState)
    {
    case QAbstractSocket::ConnectedState:
        break;

    default:    //Stop the Timer if the socket is not running
        setStreaming(false);
        break;
    }

}

uint16_t* Streamer::getDepthData()
{
    return depthBuf.udata;
}

uint16_t* Streamer::getImageData()
{
    return imageBuf.udata;
}

