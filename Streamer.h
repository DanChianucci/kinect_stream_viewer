#ifndef STREAMER_H
#define STREAMER_H

#include <QObject>
#include <QThread>
#include <QTcpSocket>

#include <stdint.h>


#define BUF_SIZE 153600

class Streamer : public QObject
{
    Q_OBJECT
public:

    enum STREAMMODE
    {
        STREAM_DEPTH,
        STREAM_IR,
        STREAM_RGB,
        STREAM_OVERLAY
    };

    enum StreamError{
        NoDataError,
        NotConnectedError,
        TimeOutError,
        InvalidData,
        UnknownError = -1
    };

    explicit Streamer(QObject *parent = 0);
    void connectToSocket(const QString& host, int port);
    void setStreaming(bool set);
    void setMode(STREAMMODE m);
    void reset();
    uint16_t* getDepthData();
    uint16_t* getImageData();

signals:
    void dataReady();
    void streamChanged(bool streaming);
    void socketChanged(QAbstractSocket::SocketState socketState);
    void socketErrorOccurred(QAbstractSocket::SocketError socketError, QString errorString);
    void streamErrorOccurred(Streamer::StreamError streamError);

public slots:


    /**
     * Tells the host to send another data packet.
     * Initiated by Timer module.
     */
    void updateData();

    /**
     * Reads data sent by host and places in data buffer.
     * \emits dataReady when buffer has a full frame
     */
    void socketRecievedData();

    /**
     * Handles any Socket Errors
     * @param socketError the error
     * @emit socketErrorOccured
     */
    void socketError(QAbstractSocket::SocketError socketError);

    /**
     * Handles State Change of the Socket
     * @param socketState
     */
    void socketStateChanged(QAbstractSocket::SocketState socketState);

private:
    QTcpSocket socket;
    bool running;

    uint32_t index;     //The position in the buffer to insert new data
    short buffNum;
    bool buffFull;      //The buffer currently being filled;

    STREAMMODE mode;
    union buffer_t
    {
        char        data[BUF_SIZE];
        uint16_t    udata[BUF_SIZE/2];
    };

    buffer_t depthBuf;
    buffer_t imageBuf;

    void fillBuffer(buffer_t &buf);

};
#endif // STREAMER_H
