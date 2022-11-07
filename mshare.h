#ifndef MSHARE_H
#define MSHARE_H

#include <QSharedMemory>
#include <QDebug>

class MShare : public QObject
{
    Q_OBJECT

  public:
    explicit MShare(QObject *parent = nullptr);

  public slots:
    void write_to_shared_memory(QVector<double> vector);
    void read_from_shared_memory();

 private:
    void detach();

signals:
    void read_data_from_shared_memory(QVector<double> vector);

  private:
    QSharedMemory shared_memory;
};

#endif // MSHARE_H
