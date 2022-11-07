QT       += core gui printsupport
QT += serialport
QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    comport.cpp \
    infowindow.cpp \
    main.cpp \
    lidar.cpp \
    mshare.cpp \
    qcustomplot.cpp \
    tcpsocket.cpp

HEADERS += \
    comport.h \
    infowindow.h \
    lidar.h \
    mshare.h \
    qcustomplot.h \
    tcpsocket.h

FORMS += \
    infowindow.ui \
    lidar.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
