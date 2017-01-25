QT       += core
QT       -= gui

TARGET = IPCameraDiscovery
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    OnvifDevice.cpp \
    OnvifDeviceDiscovery.cpp \
    wsdl_devicemgmt.cpp \
    utils.cpp

HEADERS += \
    OnvifDevice.h \
    OnvifDeviceDiscovery.h \
    wsdl_devicemgmt.h \
    utils.h \
    socket.h

LIBS += -L /home/wb12/work/SAY/onvif/KDSoap/lib -lkdsoap
INCLUDEPATH += /home/wb12/work/SAY/onvif/KDSoap/src
INCLUDEPATH += /home/wb12/work/SAY/onvif/KDSoap/include
