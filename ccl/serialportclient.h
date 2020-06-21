#ifndef SERIALPORTCLIENT_H
#define SERIALPORTCLIENT_H

#include <QObject>
#include <QSerialPort>
#include "queue/abstractqueue.h"

#define SERIALPORT_DEFAULT_BUF_SIZE 1024

typedef struct SerialPortBuffer_TAG{
    char buffer[SERIALPORT_DEFAULT_BUF_SIZE];
    qint64 len;
}SerialPortBuffer;


class SerialPortClient:public QObject
{
    Q_OBJECT
public:
    explicit SerialPortClient(const QString &portName,
                  AbstractQueue<SerialPortBuffer> *queue,
                  QObject * parent = nullptr);

    explicit SerialPortClient(const QString &portName,
                  QSerialPort::BaudRate baudRate,
                  QSerialPort::DataBits dataBits,
                  QSerialPort::Parity parity,
                  QSerialPort::StopBits stopBits,
                  QSerialPort::FlowControl flowControl,
                  AbstractQueue<SerialPortBuffer> *queue,
                  QObject * parent = nullptr);


    void start();
    void stop();

    void write(const SerialPortBuffer &buffer);

    QString portName() const;
    void setPortName(const QString &portName);

    QSerialPort::BaudRate baudRate() const;
    void setBaudRate(const QSerialPort::BaudRate &baudRate);

    QSerialPort::DataBits dataBits() const;
    void setDataBits(const QSerialPort::DataBits &dataBits);

    QSerialPort::Parity parity() const;
    void setParity(const QSerialPort::Parity &parity);

    QSerialPort::StopBits stopBits() const;
    void setStopBits(const QSerialPort::StopBits &stopBits);

    QSerialPort::FlowControl flowControl() const;
    void setFlowControl(const QSerialPort::FlowControl &flowControl);

signals:
    void startSignal();
    void stopSignal();

    void writeSignal(const SerialPortBuffer &buffer);

    void openFailure(const QString &errorString);
    void errorOccured(QSerialPort::SerialPortError error);

private slots:
    void startSlot();
    void stopSlot();

    void writeSlot(const SerialPortBuffer &buffer);

    void readyReadSlot();
    void errorOccuredSlot(QSerialPort::SerialPortError error);

private:
    QString m_portName;
    QSerialPort::BaudRate m_baudRate;
    QSerialPort::DataBits m_dataBits;
    QSerialPort::Parity m_parity;
    QSerialPort::StopBits m_stopBits;
    QSerialPort::FlowControl m_flowControl;


    QSerialPort * m_serialPort;
    AbstractQueue<SerialPortBuffer> *m_queue;
};

#endif // SERIALPORTCLIENT_H
