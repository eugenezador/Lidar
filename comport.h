#ifndef COMPORT_H
#define COMPORT_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>

class ComPort : public QObject
{
    Q_OBJECT
public:
    explicit ComPort(QObject *parent = nullptr);
    ~ComPort();

    QByteArray read_data;

public:
    QSerialPort *serial = nullptr;// указатель на область памяти для экземпляра порта

    void open_serial_port(const QString &text);
    void close_serial_port();

signals:
    void received_data(QByteArray &data);

public slots:

    void writeData(const QByteArray &data);

    void readData();

};

#endif // COMPORT_H
