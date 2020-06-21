#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QTcpSocket>
#include <QMutex>
#include <QWaitCondition>
#include <QTimer>
#include <QHostAddress>

#include "ccl/queue/abstractqueue.h"

#define TCP_DEFAULT_BUF_SIZE 1024
#define TCP_DEfAULT_RECONNECT_TIME 2000

typedef struct TCPBuffer_TAG{
    char buffer[TCP_DEFAULT_BUF_SIZE];
    qint64 len;
}TCPBuffer;

class TcpClient: public QObject
{
    Q_OBJECT
public:
    TcpClient(const QString &host,
          quint16 port,
          AbstractQueue<TCPBuffer> * queue,
          QObject * parent = nullptr);

    virtual ~TcpClient() override;

    void write(const TCPBuffer &buffer);

    void start();

    void stop();

    QString host() const;
    void setHost(const QString &host);

    quint16 port() const;
    void setPort(const quint16 &port);

signals:
    void startSignal();
    void stopSignal();

    void writeBufferSignal(const TCPBuffer &buffer);

    void unconnected();
    void connecting();
    void connected();
    void closing();

    void error(QAbstractSocket::SocketError sockeError);

private slots:
    void startSlot();
    void stopSlot();

    void writeBufferSlot(const TCPBuffer &buffer);

    void readyReadSlot();
    void stateChangedSlot(QTcpSocket::SocketState state);
    void errorSlot(QAbstractSocket::SocketError socketError);

    void timeoutSlot();

private:
    QString m_host;
    quint16 m_port;

    AbstractQueue<TCPBuffer> *m_queue;

    QTcpSocket * m_socket;
    QTimer * m_timer;

    int m_interval;
};

#endif // TCPCLIENT_H
