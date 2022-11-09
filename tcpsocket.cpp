#include "tcpsocket.h"

TcpSocket::TcpSocket(QObject *parent)
    : QObject{parent}
    , socket(new QTcpSocket(this))
{

}

TcpSocket::~TcpSocket()
{
    delete socket;
}

void TcpSocket::do_connect()
{
    connect(socket, &QTcpSocket::connected, this, &TcpSocket::connected);
    connect(socket, &QTcpSocket::disconnected, this, &TcpSocket::disconnected);
    connect(socket, &QTcpSocket::readyRead, this, &TcpSocket::readyRead);

    qDebug() << "connecting...";

    socket->connectToHost("192.168.2.10", 800);

    if(!socket->waitForConnected(4000))
    {
        qDebug() << "Error: " << socket->errorString();
    }
}

void TcpSocket::do_disconnect()
{
    socket->disconnectFromHost();
}

void TcpSocket::connected()
{
    is_connect = true;

    qDebug() << "connected...";

    socket->write("");
}

void TcpSocket::disconnected()
{
    is_connect = false;

    qDebug() << "disconnected...";
}

void TcpSocket::readyRead()
{
    received_data.clear();

    received_data.append(socket->readAll());

    emit data_from_lidar(received_data);
}

