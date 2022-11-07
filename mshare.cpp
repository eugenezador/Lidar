#include "mshare.h"
#include <QFileDialog>
#include <QBuffer>

MShare::MShare(QObject *parent) : QObject(parent)
  , shared_memory("MySharedMemory")
{

}

void MShare::write_to_shared_memory(QVector<double> vector)
{
    QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);
    QDataStream out(&buffer);

    //write the vector object
    out << vector;

    int size = buffer.size();
    if (!shared_memory.create(size)) {
        qDebug() << "Unable to create shared memory segment.";
        return;
    }
    shared_memory.lock();
    char *to = (char*)shared_memory.data();
    const char *from = buffer.data().data();
    memcpy(to, from, qMin(shared_memory.size(), size));
    shared_memory.unlock();
}

void MShare::read_from_shared_memory()
{
    shared_memory.attach();
    QBuffer buffer;
    QDataStream in(&buffer);

    shared_memory.lock();
    buffer.setData((char*)shared_memory.constData(), shared_memory.size());
    buffer.open(QBuffer::ReadOnly);

    QVector<double> vector;
    in>>vector;
    shared_memory.unlock();
    qDebug() << vector;
    emit read_data_from_shared_memory(vector);
}

void MShare::detach()
{
    if (!shared_memory.detach())
        qDebug() << "Unable to detach from shared memory.";
}
