#include "serialportclient.h"
#include <QDebug>

SerialPortClient::SerialPortClient(const QString &portName,
                   AbstractQueue<SerialPortBuffer> * queue,
                   QObject *parent)
    : QObject(parent),
      m_portName(portName),
      m_baudRate(QSerialPort::Baud9600),
      m_dataBits(QSerialPort::Data8),
      m_parity(QSerialPort::Parity::NoParity),
      m_stopBits(QSerialPort::StopBits::OneStop),
      m_flowControl(QSerialPort::FlowControl::NoFlowControl),
      m_queue(queue)
{
    m_serialPort = new QSerialPort(this);
    connect(m_serialPort,&QSerialPort::readyRead,this,&::SerialPortClient::readyReadSlot);
    connect(m_serialPort,&QSerialPort::errorOccurred,this,&SerialPortClient::errorOccuredSlot);
    connect(this,&SerialPortClient::startSignal,this,&SerialPortClient::startSlot);
    connect(this,&SerialPortClient::stopSignal,this,&SerialPortClient::stopSlot);
    connect(this,&SerialPortClient::writeSignal,this,&SerialPortClient::writeSlot);
}

SerialPortClient::SerialPortClient(const QString &portName,
                                   QSerialPort::BaudRate baudRate,
                                   QSerialPort::DataBits dataBits,
                                   QSerialPort::Parity parity,
                                   QSerialPort::StopBits stopBits,
                                   QSerialPort::FlowControl flowControl,
                                   AbstractQueue<SerialPortBuffer> *queue,
                                   QObject *parent)
    : QObject(parent),
      m_portName(portName),
      m_baudRate(baudRate),
      m_dataBits(dataBits),
      m_parity(parity),
      m_stopBits(stopBits),
      m_flowControl(flowControl),
      m_queue(queue)
{

}

void SerialPortClient::start()
{
    emit startSignal();
}

void SerialPortClient::stop()
{
    emit stopSignal();
}

void SerialPortClient::startSlot()
{
    if(m_serialPort->isOpen()){
        m_serialPort->close();
    }

    m_serialPort->setPortName(m_portName);
    m_serialPort->setBaudRate(m_baudRate);
    m_serialPort->setDataBits(m_dataBits);
    m_serialPort->setParity(m_parity);
    m_serialPort->setStopBits(m_stopBits);
    m_serialPort->setFlowControl(m_flowControl);
    if(!m_serialPort->open(QIODevice::ReadWrite)){
        // open error
        QString errorString = m_serialPort->errorString();
        qDebug()<<"Serial Port open failure! "<<errorString;
        emit openFailure(errorString);
    }
}

void SerialPortClient::stopSlot()
{
    if(m_serialPort->isOpen()){
        m_serialPort->close();
    }
}

void SerialPortClient::writeSlot(const SerialPortBuffer &buffer)
{
    qint64 len = 0;
    while(len < buffer.len){
        qint64 lenTmp = m_serialPort->write(buffer.buffer + len,buffer.len - len);
        if(lenTmp < 0){
            qDebug()<<"Write buffer failure! buffer: "<<
                  QByteArray(buffer.buffer + len,
                         static_cast<int>(buffer.len - len)).toHex();
            break;

        }
        len += lenTmp;
    }
}

void SerialPortClient::readyReadSlot()
{
    SerialPortBuffer *buffer = m_queue->peekWriteable();
    if(!buffer){
        qDebug()<<"Peek write buffer failure! Please check queue is abort!";
        return;
    }
    buffer->len = m_serialPort->read(buffer->buffer,SERIALPORT_DEFAULT_BUF_SIZE);
    if(buffer->len < 0){
        qDebug()<<"SerialPort read failure! Error: "<< m_serialPort->errorString();
        m_queue->next(buffer);
        return;
    }
    m_queue->push(buffer);
}

void SerialPortClient::errorOccuredSlot(QSerialPort::SerialPortError error)
{
    qDebug()<<"Serial Prot Error: "<<error;
    emit errorOccured(error);
}

QSerialPort::FlowControl SerialPortClient::flowControl() const
{
    return m_flowControl;
}

void SerialPortClient::setFlowControl(const QSerialPort::FlowControl &flowControl)
{
    m_flowControl = flowControl;
}

QSerialPort::StopBits SerialPortClient::stopBits() const
{
    return m_stopBits;
}

void SerialPortClient::setStopBits(const QSerialPort::StopBits &stopBits)
{
    m_stopBits = stopBits;
}

QSerialPort::Parity SerialPortClient::parity() const
{
    return m_parity;
}

void SerialPortClient::setParity(const QSerialPort::Parity &parity)
{
    m_parity = parity;
}

QSerialPort::DataBits SerialPortClient::dataBits() const
{
    return m_dataBits;
}

void SerialPortClient::setDataBits(const QSerialPort::DataBits &dataBits)
{
    m_dataBits = dataBits;
}

QSerialPort::BaudRate SerialPortClient::baudRate() const
{
    return m_baudRate;
}

void SerialPortClient::setBaudRate(const QSerialPort::BaudRate &baudRate)
{
    m_baudRate = baudRate;
}

QString SerialPortClient::portName() const
{
    return m_portName;
}

void SerialPortClient::setPortName(const QString &portName)
{
    m_portName = portName;
}
