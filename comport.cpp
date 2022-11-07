#include "comport.h"

ComPort::ComPort(QObject *parent) : QObject(parent) ,
    serial(new QSerialPort(this))
{
    connect(serial, &QSerialPort::readyRead, this, &ComPort::readData);
}

ComPort::~ComPort()
{
    serial->close();
    delete serial;
}

void ComPort::open_serial_port(const QString &text)
{
    serial->setPortName(text);

    serial->open(QIODevice::ReadWrite);
    serial->setBaudRate(QSerialPort::Baud38400);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
}

void ComPort::close_serial_port()
{
    if (serial->isOpen())
        serial->close();
}

void ComPort::writeData(const QByteArray &data)
{
    if(serial->isOpen() == true){
        serial->write(data);
        //serial->write("\r");
        serial->waitForBytesWritten();

        qDebug() << "write: " << data;
    }
    else
    {
        qDebug() << "not open";
    }
}

void ComPort::readData()
{
    read_data.append(serial->readAll());

    if(strstr(read_data, "\r")){

        qDebug() << "full read_data: " << read_data;

        emit received_data(read_data);

        read_data.clear();
    }
//    else qDebug() << "not full read_data: " << read_data;
}
