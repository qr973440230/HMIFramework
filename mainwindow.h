#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>

#include "ccl/serialportclient.h"
#include "ccl/tcpclient.h"
#include "ccl/udpclient.h"
#include "ccl/queue/waitqueue.h"



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class TcpParseThread;
class UdpParseThread;
class SerialPortParseThread;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    WaitQueue<TCPBuffer> tcpQueue;
    WaitQueue<UDPBuffer> udpQueue;
    WaitQueue<SerialPortBuffer> serialPortQueue;
    QThread * thread;

    TcpParseThread * tcpParseThread;
    UdpParseThread * udpParseThread;
    SerialPortParseThread * serialPortParseThread;
};

class TcpParseThread: public QThread{

public:
    TcpParseThread(AbstractQueue<TCPBuffer> * queue,QObject * parent = nullptr);

protected:
    void run() override;

private:
    AbstractQueue<TCPBuffer> * m_queue;
};

class UdpParseThread: public QThread{

public:
    UdpParseThread(AbstractQueue<UDPBuffer> * queue,QObject * parent = nullptr);

protected:
    void run() override;

private:
    AbstractQueue<UDPBuffer> * m_queue;
};

class SerialPortParseThread: public QThread{

public:
    SerialPortParseThread(AbstractQueue<SerialPortBuffer> * queue,QObject * parent = nullptr);

protected:
    void run() override;

private:
    AbstractQueue<SerialPortBuffer> * m_queue;
};

#endif // MAINWINDOW_H
