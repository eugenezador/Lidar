#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QDebug>

class TcpSocket : public QObject
{
    Q_OBJECT
public:
    explicit TcpSocket(QObject *parent = nullptr);
    ~TcpSocket();

    void do_connect();
    void do_disconnect();

    bool is_connect = false; // переменная сообщает статус подключения а классе lidar

signals:
    void data_from_lidar(QByteArray received_data);

public slots:
    void connected();
    void disconnected();
    void readyRead();

private:
    QTcpSocket *socket;

    QByteArray received_data;
};

#endif // TCPSOCKET_H
