#include "udpclient.h"

UdpClient::UdpClient(quint16 port,
             AbstractQueue<UDPBuffer> *queue,
             QObject *parent)
    :QObject(parent),
      m_host(QHostAddress::LocalHost),
      m_port(port),
      m_queue(queue)
{
    m_socket = new QUdpSocket(this);

    connect(this,&UdpClient::startSignal,this,&UdpClient::startSlot);
    connect(this,&UdpClient::stopSignal,this,&UdpClient::stopSlot);

    connect(this,&UdpClient::writeSignal,this,&UdpClient::writeBufferSlot);

    connect(m_socket,&QUdpSocket::readyRead,this,&UdpClient::readyReadSlot);
    connect(m_socket,&QUdpSocket::stateChanged,this,&UdpClient::stateChangedSlot);
    connect(m_socket,QOverload<QAbstractSocket::SocketError>::of(&QUdpSocket::error),
        this,&UdpClient::errorSlot);

}

UdpClient::UdpClient(const QHostAddress &host,
             quint16 port,
             AbstractQueue<UDPBuffer> *queue,
             QObject *parent)
    :QObject(parent),
      m_host(host),
      m_port(port),
      m_queue(queue)
{
    m_socket = new QUdpSocket(this);

    connect(this,&UdpClient::startSignal,this,&UdpClient::startSlot);
    connect(this,&UdpClient::stopSignal,this,&UdpClient::stopSlot);

    connect(this,&UdpClient::writeSignal,this,&UdpClient::writeBufferSlot);

    connect(m_socket,&QUdpSocket::readyRead,this,&UdpClient::readyReadSlot);
    connect(m_socket,&QUdpSocket::stateChanged,this,&UdpClient::stateChangedSlot);
    connect(m_socket,QOverload<QAbstractSocket::SocketError>::of(&QUdpSocket::error),
        this,&UdpClient::errorSlot);

}

void UdpClient::write(const UDPBuffer &buffer)
{
    emit writeSignal(buffer);
}

void UdpClient::start()
{
    emit startSignal();
}

void UdpClient::stop()
{
    emit stopSignal();
}

void UdpClient::startSlot()
{
    if(m_socket->state() == QAbstractSocket::BoundState){
        m_socket->close();
    }
    m_socket->bind(m_host,m_port);
}

void UdpClient::stopSlot()
{
    m_socket->close();
}

void UdpClient::writeBufferSlot(const UDPBuffer &buffer)
{
    qint64 len = m_socket->writeDatagram(buffer.buffer,buffer.len,
                         buffer.addres,buffer.port);
    if(len < 0){
        qDebug()<<"Write buffer failure! buffer: "<<
              QByteArray(buffer.buffer + len,
                     static_cast<int>(buffer.len - len)).toHex()<<
              " Host: "<<buffer.addres<<
              " Port: "<<buffer.port;
    }
}

void UdpClient::readyReadSlot()
{
    if(m_socket->hasPendingDatagrams()){
        UDPBuffer * buffer = m_queue->peekWriteable();
        if(!buffer){
            qDebug()<<"Peek write buffer failure! Please check queue is abort!";
            return;
        }
        buffer->len = m_socket->readDatagram(buffer->buffer,UDP_DEFAULT_BUF_SIZE,&buffer->addres,&buffer->port);
        if(buffer->len < 0){
            qDebug()<<"Socket read failure! Error: "<< m_socket->errorString();
            m_queue->next(buffer);
            return;
        }
        m_queue->push(buffer);
    }
}

void UdpClient::stateChangedSlot(QAbstractSocket::SocketState state)
{
    qDebug()<<"UDPClient state changed! Current state: " << state;
}

void UdpClient::errorSlot(QAbstractSocket::SocketError socketError)
{
    qDebug()<<"Udp Socket Error: "<< socketError;
    emit error(socketError);
}

quint16 UdpClient::port() const
{
    return m_port;
}

void UdpClient::setPort(const quint16 &port)
{
    m_port = port;
}

QHostAddress UdpClient::host() const
{
    return m_host;
}

void UdpClient::setHost(const QHostAddress &host)
{
    m_host = host;
}
