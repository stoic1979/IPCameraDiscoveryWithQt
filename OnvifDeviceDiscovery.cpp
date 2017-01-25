#include <sys/types.h>
#include <socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>

#include "OnvifDeviceDiscovery.h"
#include "OnvifDevice.h"

#include "wsdl_devicemgmt.h"

static const int BUFFER_MAX = 65536;
static const char OnvifProbe[] = "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:a=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\"><s:Header><a:Action s:mustUnderstand=\"1\">http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe</a:Action><a:MessageID>urn:uuid:1d3cf678-1234-1234-12a456789abc</a:MessageID><a:ReplyTo><a:Address>http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</a:Address></a:ReplyTo><a:To s:mustUnderstand=\"1\">urn:schemas-xmlsoap-org:ws:2005:04:discovery</a:To></s:Header><s:Body><Probe xmlns=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\"><d:Types xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" xmlns:dp0=\"http://www.onvif.org/ver10/network/wsdl\" xmlns:dp1=\"http://www.onvif.org/ver10/device/wsdl\">dp0:NetworkVideoTransmitter dp1:Device</d:Types></Probe></s:Body></s:Envelope>";


OnvifDeviceDiscovery::OnvifDeviceDiscovery(QObject *parent) : QObject(parent) {
    connect(this,SIGNAL(CameraDetected()),this,SLOT(onCameraDetected()));
}

OnvifDeviceDiscovery::~OnvifDeviceDiscovery(){}

void OnvifDeviceDiscovery::IPCameraDiscovery(){}

bool OnvifDeviceDiscovery::StartDiscovery() {
    isRunning = true;
    recvBuffer = new char[BUFFER_MAX];

    SetupSocket();
    SendProbe();

    DeviceServiceEndpoint =  OnvifCameraListener();
    return true;

}

bool OnvifDeviceDiscovery::StopDiscovery() {
    return true;
}

void OnvifDeviceDiscovery::SetupSocket() {

    sockaddr_in localAddr;
    ip_mreq mreq;
    char broadcastIP[] = "239.255.255.250";
    int timeout = 5000;

    // setup multicast and subscribe to group
    mreq.imr_multiaddr.s_addr = inet_addr(broadcastIP);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    memset(&localAddr, 0, sizeof(sockaddr_in));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = INADDR_ANY;
    localAddr.sin_port = 0;

    // create socket
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        qDebug()<<"Socket create failed!";

    // subscribe to multicast group
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&mreq, sizeof(mreq)) < 0)
        qDebug() << "Multicast Membership failed!";

    // set option to reuse address
    if (ReuseAddress(sock) < 0)
        qDebug() << "Socket Reuse addr failed!";

    // set receive timeout option on the socket
    if (SetSocketTimeout(sock, timeout) < 0)
        qDebug() << "Socket Timeout failed!";

    // bind the socket to local address
    if (bind(sock, (sockaddr *)&localAddr, sizeof(sockaddr)) == -1)
        qDebug() << "Socket bind failed!";
}

bool OnvifDeviceDiscovery::DetectOnvifCameras() {

    // send out an onvif probe message to the network
    sockaddr_in broadcastAddr;
    const char *broadcastIP = "239.255.255.250";
    const int broadcastPort = 3702;

    memset(&broadcastAddr, 0, sizeof(broadcastAddr));

    // set up multi-cast address
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_addr.s_addr = inet_addr(broadcastIP);
    broadcastAddr.sin_port = htons(broadcastPort);

    int bufferLenOne = strlen(OnvifProbe);

    for (int i = 0; i < 1; i++){
        if (sendto(sock, OnvifProbe, bufferLenOne, 0, (struct sockaddr *) &broadcastAddr, sizeof(broadcastAddr)) != bufferLenOne) {
            qDebug() << "Send probe failed!";
            return false;
        }
    }

    //networkManager->incrementCameraValues();
    return true;
}


QString OnvifDeviceDiscovery::OnvifCameraListener() {
    sockaddr_in fromAddr;
    memset(&fromAddr, 0, sizeof(fromAddr));

    while(isRunning)
    {
        socklen_t slen = sizeof(sockaddr);

        int val = recvfrom(sock, recvBuffer, BUFFER_MAX, 0, (sockaddr *) &fromAddr, &slen);
        if (val < 0)
            break;

        recvBuffer[val] = '\0';

        // check to see if the message is the correct ONVIF format,
        // and if so store the IP address for later use
        if (fromAddr.sin_addr.s_addr != 0){
            KDSoapMessageReader reader;
            KDSoapMessage replyMessage;
            KDSoapHeaders replyHeader;

            this->CameraDetected();
            reader.xmlToMessage(recvBuffer, &replyMessage, 0, &replyHeader);

            if(replyMessage.name() == "ProbeMatches"){
                QString deviceServiceEndpoint = replyMessage.arguments().child("ProbeMatch").childValues().child("XAddrs").value().toString();
                return deviceServiceEndpoint;
            }
        }
    }

    if (isRunning)
        QMetaObject::invokeMethod(this, "OnvifCameraListener", Qt::QueuedConnection);
    else
    {
        delete [] recvBuffer;
        // close(sock);
        qDebug() << "Onvif Camera Listener Stopped!";
    }
}

void OnvifDeviceDiscovery::SendProbe() {
    DetectOnvifCameras();
}

bool OnvifDeviceDiscovery::IsOnvifDevice(QString msg) {
    return  msg.contains("onvif") && msg.contains("NetworkVideoTransmitter");
}

void OnvifDeviceDiscovery::onCameraDetected() {
    qDebug() << "Camera detected";
}
