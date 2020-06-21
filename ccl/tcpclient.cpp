#include "tcpclient.h"
#include <QDebug>
#include <QThread>
#include <QTimer>

TcpClient::TcpClient(const QString &host,
             quint16 port,
             AbstractQueue<TCPBuffer> *queue,
             QObject *parent)
    :QObject(parent),
      m_host(host),
      m_port(port),
      m_queue(queue),
      m_socket(nullptr),
      m_timer(nullptr),
      m_interval(TCP_DEfAULT_RECONNECT_TIME)
{
    m_socket = new QTcpSocket(this);
    m_timer = new QTimer(this);

    // socket
    qRegisterMetaType<QTcpSocket::SocketState>("QTcpSocket::SocketState");
    connect(m_socket,&QTcpSocket::stateChanged,this,&TcpClient::stateChangedSlot);
    connect(m_socket,&QTcpSocket::readyRead,this,&TcpClient::readyReadSlot);
    connect(m_socket,QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
        this,&TcpClient::errorSlot);

    // timer
    connect(m_timer,&QTimer::timeout,this,&TcpClient::timeoutSlot);

    // self
    connect(this,&TcpClient::startSignal,this,&TcpClient::startSlot);
    connect(this,&TcpClient::stopSignal,this,&TcpClient::stopSlot);

    // self write
    connect(this,&TcpClient::writeBufferSignal,this,&TcpClient::writeBufferSlot);
}

TcpClient::~TcpClient()
{

}

void TcpClient::write(const TCPBuffer &buffer)
{
    emit writeBufferSignal(buffer);
}

void TcpClient::start()
{
    emit startSignal();
}

void TcpClient::stop()
{
    emit stopSignal();
}

void TcpClient::startSlot()
{
    if(m_socket->state() == QAbstractSocket::ConnectedState){
        m_socket->close();
    }
    m_timer->start(m_interval);
    m_socket->connectToHost(m_host,m_port);
}

void TcpClient::stopSlot()
{
    m_timer->stop();
    m_socket->close();
}

void TcpClient::writeBufferSlot(const TCPBuffer &buffer)
{
    qint64 len = 0;

    while(len < buffer.len){
        qint64 lenTmp = m_socket->write(buffer.buffer + len,buffer.len - len);
        if(lenTmp < 0){
            qDebug()<<"Write buffer failure! Error:"<<m_socket->errorString()<<
                  " buffer: "<<QByteArray(buffer.buffer + len,
                              static_cast<int>(buffer.len - len)).toHex();
            break;
        }
        len += lenTmp;
    }
}

void TcpClient::readyReadSlot()
{
    TCPBuffer *buffer = m_queue->peekWriteable();
    if(!buffer){
        qDebug()<<"Peek write buffer failure! Please check queue is abort!";
        return;
    }

    buffer->len = m_socket->read(buffer->buffer,TCP_DEFAULT_BUF_SIZE);
    if(buffer->len < 0){
        qDebug()<<"Socket read failure! Error: "<< m_socket->errorString();
        m_queue->next(buffer);
        return;
    }

    m_queue->push(buffer);
}

void TcpClient::stateChangedSlot(QAbstractSocket::SocketState state)
{
    qDebug()<<"TcpClient state changed! Current state: " << state;

    switch (state) {
    case QTcpSocket::UnconnectedState:
        emit unconnected();
        break;
    case QTcpSocket::ConnectingState:
        emit connecting();
        break;
    case QTcpSocket::ConnectedState:
        emit connected();
        break;
    case QTcpSocket::ClosingState:
        emit closing();
        break;

    default:
        break;
    }
}

void TcpClient::errorSlot(QAbstractSocket::SocketError socketError)
{
    qDebug()<<"Tcp Socket Error: "<< socketError;
    emit error(socketError);
}

void TcpClient::timeoutSlot()
{
    if(m_socket->state() == QTcpSocket::UnconnectedState){
        m_socket->connectToHost(m_host,m_port);
        if(!m_socket->waitForConnected(m_interval/2)){
            qDebug()<<"Reconnect server failure!"<<
                  " Host: "<<m_host<<
                  " Port: "<<m_port;
        }else{
            qDebug()<<"Reconnect server success!"<<
                  " Host: "<<m_host<<
                  " Port: "<<m_port;
        }
    }
}

quint16 TcpClient::port() const
{
    return m_port;
}

void TcpClient::setPort(const quint16 &port)
{
    m_port = port;
}

QString TcpClient::host() const
{
    return m_host;
}

void TcpClient::setHost(const QString &host)
{
    m_host = host;
}
