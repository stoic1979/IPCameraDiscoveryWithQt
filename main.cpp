#include <QCoreApplication>
#include <QDebug>

#include "OnvifDevice.h"
#include "OnvifDeviceDiscovery.h"
#include "utils.h"

#include "wsdl_devicemgmt.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    OnvifDeviceDiscovery onvifDeviceDiscovery;

    onvifDeviceDiscovery.StartDiscovery();
    QString endpoint = onvifDeviceDiscovery.DeviceServiceEndpoint;
    OnvifDevice onvifDevice(endpoint);

    if (endpoint.isNull() ||endpoint.isEmpty())
    {
        qDebug() << "No device service endpoint!";
        return -1;
    }

    onvifDevice.setAuthentication("admin","admin");

    // getting the device info
    QHash <QString, QVariant> DeviceInfo = onvifDevice.getDeviceInformation();

    qDebug("\n\n--------------------------------------------------------------------");
    qDebug("|                   Camera Info                                    |");
    qDebug("--------------------------------------------------------------------\n");

    qDebug()<<" Manufacturer          :        "<<DeviceInfo["Manufacturer"].toString();
    qDebug()<<" Model No              :        "<<DeviceInfo["Model"].toString();
    qDebug()<<" Serial No             :        "<<DeviceInfo["Serial"].toString();
    qDebug()<<" Hardware ID           :        "<<DeviceInfo["HardwareID"].toString();
    qDebug()<<" Firmware Version      :        "<<DeviceInfo["Firmware"].toString();


    // getting available resolution for encoder JPEG
    QString encoderName = "H264";
    QList <QSize> ResolutionList =  onvifDevice.getAvailableResolution(encoderName);
    int i = 0;

    qDebug()<<"\n\n--------------------------------------------------------------------";
    qDebug()<<"|                Available Resolutions                             |";
    qDebug()<<"--------------------------------------------------------------------\n";
    qDebug()<<"Enocoder : "<<encoderName;
    foreach (QSize Resolution, ResolutionList) {
        qDebug()<< ++i <<". Width x Height      :        " << Resolution.rwidth() << " x " << Resolution.rheight();
    }

    //gettig frame rate, bitrate, resolution
    QSize resVal = onvifDevice.getResolution();
    int Brate = onvifDevice.getBitrate();
    int FRate = onvifDevice.getFps();

    qDebug()<<"\n\n--------------------------------------------------------------------";
    qDebug()<<"|                     Current Settings of Camera                    |";
    qDebug()<<"--------------------------------------------------------------------\n";
    qDebug()<< "Resolution :  Width x Height     :         " << resVal.rwidth()<<"x"<< resVal.rheight();
    qDebug()<< "Frame Rate                       :         " << FRate;
    qDebug()<< "Bitrate                          :         " << Brate;

    qDebug()<<"\n\n--------------------------------------------------------------------";
    qDebug()<<"|                      Camera RTSP PATH                             |";
    qDebug()<<"--------------------------------------------------------------------\n";
    qDebug()<< "\n RTSP PATH   :   " << onvifDevice.getRtspPath()<<"\n";

    qDebug()<<"\n--------------------------------------------------------------------";
    qDebug()<<"|                      Going to set new settings                   |";
    qDebug()<<"--------------------------------------------------------------------\n";


    QSize resolution(1920, 1080);
    // settig the resolution
    onvifDevice.setResolution(resolution);

    // setting frame rate
    onvifDevice.setFps(22);//10 to 30

    //setting the bitrate
    onvifDevice.setBitrate(2300);

    return a.exec();
}
