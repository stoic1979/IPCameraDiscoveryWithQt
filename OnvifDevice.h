#ifndef ONVIFDEVICE_H
#define ONVIFDEVICE_H

#include "wsdl_devicemgmt.h"
#include <QHash>
#include <QSize>

using namespace dm;

class OnvifDevice : public QObject {
    Q_OBJECT

    public:

        explicit OnvifDevice(QString deviceServiceEndpoint, QObject *parent = 0);
        ~OnvifDevice();

        QString getRtspPath();
        QSize getResolution();
        QSize setResolution(QSize resolution);

        int getFps();
        int setFps(int Fps);
        int getBitrate();
        int setBitrate(int bitrate);

        QHash <QString, QVariant>  getDeviceInformation();
        void setAuthentication(QString username, QString password);
        QList<QSize> getAvailableResolution(QString VideoEncoderName);

    private:

        QString DeviceServiceEndpoint; // Device Service End point
        QString mediaPoint;            // Media Service End point of Device
        QString EventPint;             // Event point
        QString Username;
        QString Password;

        QHash <QString, QVariant> VideoEncoderSettings;

        TDS__GetDeviceInformationResponse getDeviceInfo();

        KDSoapMessage getAuthenticationHeader(QString username, QString password);
        KDSoapMessage getMediaProfile(QString MediaServiceEndpoint);
        KDSoapMessage getStreamURI(QString MediaServiceEndpoint);

        KDSoapMessage setVideoEncoderConfiguration(QString MediaServiceEndpoint);
        KDSoapMessage getVideoEncoderConfiguration(QString MediaServiceEndpoint);
        KDSoapMessage getVideoEncoderConfigurationOptions(QString MediaServiceEndpoint);
        KDSoapValue getToken(QLatin1String TokenName, QVariant TokenValue, bool QualifiedValue, QString NamespaceURI);

        void ConnectToDeviceService();
        void DeviceError(QString Error);

signals:
        void loadVideoEncoderDefaultSettings();

        private slots:
            void onLoadVideoEncoderDefaultSettings();
};

#endif // ONVIFDEVICE_H
