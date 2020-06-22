#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    thread = new QThread(this);
    thread->start();

    UdpClient * udpClient = new UdpClient(8888,&udpQueue);
    TcpClient * tcpClient = new TcpClient("127.0.0.1",8765,&tcpQueue);
    SerialPortClient * serialPortClient = new SerialPortClient("COM1",&serialPortQueue);

    udpClient->moveToThread(thread);
    udpClient->start();
    tcpClient->moveToThread(thread);
    tcpClient->start();
    serialPortClient->moveToThread(thread);
    serialPortClient->start();

    tcpParseThread = new TcpParseThread(&tcpQueue,this);
    tcpParseThread->start();
    udpParseThread = new UdpParseThread(&udpQueue,this);
    udpParseThread->start();
    serialPortParseThread = new SerialPortParseThread(&serialPortQueue,this);
    serialPortParseThread->start();
}

MainWindow::~MainWindow()
{
    delete ui;

}

TcpParseThread::TcpParseThread(AbstractQueue<TCPBuffer> *queue,
                               QObject *parent)
    :QThread(parent),m_queue(queue)
{

}

void TcpParseThread::run()
{
    while(!isInterruptionRequested()){
        TCPBuffer *buffer = m_queue->peekReadable(2000);
        if(!buffer){
            qDebug()<<"Peek readable TcpBuffer timeout";
            continue;
        }

        qDebug()<<"TcpBuffer: "<<QByteArray(buffer->buffer,
                                            static_cast<int>(buffer->len)).toHex();

        m_queue->next(buffer);
    }
}

UdpParseThread::UdpParseThread(AbstractQueue<UDPBuffer> *queue, QObject *parent)
    :QThread(parent),m_queue(queue)
{

}

void UdpParseThread::run()
{
    while(!isInterruptionRequested()){
        UDPBuffer *buffer = m_queue->peekReadable(2000);
        if(!buffer){
            qDebug()<<"Peek readable UdpBuffer timeout!";
            continue;
        }
        qDebug()<<"UdpBuffer: "<<QByteArray(buffer->buffer,
                                            static_cast<int>(buffer->len)).toHex();
        m_queue->next(buffer);
    }
}

SerialPortParseThread::SerialPortParseThread(AbstractQueue<SerialPortBuffer> *queue,
                                             QObject *parent)
    :QThread(parent),m_queue(queue)
{

}

void SerialPortParseThread::run()
{
    while(!isInterruptionRequested()){
        SerialPortBuffer *buffer = m_queue->peekReadable(2000);
        if(!buffer){
            qDebug()<<"Peek readable SerialPortBuffer timeout";
            continue;
        }

        qDebug()<<"SerialPortBuffer: "<<QByteArray(buffer->buffer,
                                                   static_cast<int>(buffer->len)).toHex();
        m_queue->next(buffer);
    }
}
