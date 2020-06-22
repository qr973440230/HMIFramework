#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QUdpSocket>
#include "queue/abstractqueue.h"

#define UDP_DEFAULT_BUF_SIZE 1024

typedef struct UDPBuffer_TAG{
    char buffer[UDP_DEFAULT_BUF_SIZE];
    qint64 len;
    QHostAddress addres;
    quint16 port;
}UDPBuffer;

class UdpClient:public QObject
{
    Q_OBJECT
public:
    explicit UdpClient(quint16 port,
               AbstractQueue<UDPBuffer> *queue,
               QObject * parent = nullptr);

    explicit UdpClient(const QHostAddress &host,
               quint16 port,
               AbstractQueue<UDPBuffer> *queue,
               QObject * parent = nullptr);

    void write(const UDPBuffer &buffer);

    void start();
    void stop();

    QHostAddress host() const;
    void setHost(const QHostAddress &host);

    quint16 port() const;
    void setPort(const quint16 &port);

signals:
    void startSignal();
    void stopSignal();

    void writeSignal(const UDPBuffer &buffer);
    void error(QAbstractSocket::SocketError socketError);

private slots:
    void startSlot();
    void stopSlot();

    void writeBufferSlot(const UDPBuffer &buffer);

    void readyReadSlot();
    void stateChangedSlot(QUdpSocket::SocketState state);
    void errorSlot(QAbstractSocket::SocketError socketError);

private:
    QHostAddress m_host;
    quint16 m_port;

    AbstractQueue<UDPBuffer> *m_queue;

    QUdpSocket * m_socket;
};

#endif // UDPCLIENT_H
