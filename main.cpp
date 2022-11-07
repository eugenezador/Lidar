#include "lidar.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Lidar w;
    w.show();

    return a.exec();
}
