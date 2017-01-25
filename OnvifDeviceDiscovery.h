#ifndef ONVIFDEVICEDISCOVERY_H
#define ONVIFDEVICEDISCOVERY_H

#include "OnvifDevice.h"
#include <QDebug>
#include "wsdl_devicemgmt.h"
#include <KDSoapClient/KDSoapMessageReader_p.h>

using namespace dm;

class OnvifDeviceDiscovery : public QObject
{
    Q_OBJECT
public:
    explicit OnvifDeviceDiscovery(QObject *parent = 0);
    ~OnvifDeviceDiscovery();

private:
   int sock;
   char *recvBuffer;
   int probeTimerId;

   void SetupSocket();
   bool IsOnvifDevice(QString msg);
   bool DetectOnvifCameras();
   QString OnvifCameraListener();
   void SendProbe();
   bool isStarting;
   bool isRunning;
   void IPCameraDiscovery();

public:
    QString DeviceServiceEndpoint;

   bool StartDiscovery();
   bool StopDiscovery();

signals:
  void CameraDetected();

public slots:
  void onCameraDetected();
};

#endif // ONVIFDEVICEDISCOVERY_H
