/*
 * The OnvifDevice class have the methods use to communicate with the IP camera. This class
 * contains the methods for loading default settings, generating request message for any
 * service such as getting video encoder configuration settings, gettings stream URI etc.
 * This class also contains the methods to setting various settings of the IP camera
 */

#include <QDebug>

#include "OnvifDevice.h"
#include "socket.h"
#include "utils.h"


OnvifDevice::OnvifDevice(QString deviceServiceEndpoint, QObject *parent) : QObject(parent) {
    this->DeviceServiceEndpoint  = deviceServiceEndpoint;
    connect(this,SIGNAL(loadVideoEncoderDefaultSettings()),this,SLOT(onLoadVideoEncoderDefaultSettings()));
}

OnvifDevice::~OnvifDevice() {}

// function for returning error
void OnvifDevice::DeviceError(QString Error) {
    qDebug()<< "\n "<<Error<<" : "<<"Please check username and password";
}

// setting authentication for user
void OnvifDevice::setAuthentication(QString username, QString password) {
    this->Username = username;
    this->Password = password;
    loadVideoEncoderDefaultSettings();
}

// Returning token with given name, value and namespaceURI
KDSoapValue OnvifDevice::getToken(QLatin1String TokenName, QVariant TokenValue, bool QualifiedValue, QString NamespaceURI) {
    KDSoapValue Token(TokenName,TokenValue);
    Token.setNamespaceUri(NamespaceURI);
    Token.setQualified(QualifiedValue);
    return Token;
}

// Loading current VideoEncoderConfiguration settings of camera
void OnvifDevice::onLoadVideoEncoderDefaultSettings() {
    // connect to device Service
    this->ConnectToDeviceService();

    // These settings will be Given to SetVideoEncoderConfiguration as Default settings
    KDSoapMessage mediaProfile =  this->getMediaProfile(mediaPoint);

    if(mediaProfile.name() == "GetProfilesResponse")
    {
        QList<KDSoapValue> profiles = mediaProfile.arguments();
        foreach(KDSoapValue profile, profiles){
            if(profile.childValues().size() > 2){
                VideoEncoderSettings["RWidth"] =  profile.childValues().child("VideoEncoderConfiguration").childValues().child("Resolution").childValues().child("Width") .value().toInt();
                VideoEncoderSettings["RHeight"] =  profile.childValues().child("VideoEncoderConfiguration").childValues().child("Resolution").childValues().child("Height") .value().toInt();
                VideoEncoderSettings["FRate"]  = profile.childValues().child("VideoEncoderConfiguration").childValues().child("RateControl").childValues().child("FrameRateLimit") .value().toInt();
                VideoEncoderSettings["BRate"]  =  profile.childValues().child("VideoEncoderConfiguration").childValues().child("RateControl").childValues().child("BitrateLimit") .value().toInt();
                VideoEncoderSettings["EncoderConfigTokenValue"] = profile.childValues().child("VideoEncoderConfiguration").childValues().attributes().at(0).value().toString();
                VideoEncoderSettings["VideoEncoderConfigurationMain"] = profile.childValues().child("VideoEncoderConfiguration").childValues().child("Name").value();
                VideoEncoderSettings["MediaProfileTokenValue"] =  profile.childValues().attributes().at(1).value();
                return;
            }
        }
    }
    else
        this->DeviceError("Camera default settings Can`t be loaded.");
}

// This function will return the RTSP
QString OnvifDevice::getRtspPath() {
    KDSoapMessage getStreamUriResponse =  this->getStreamURI(mediaPoint);
    QString rtspPath("Error : no RSTP Path");

    if(getStreamUriResponse.name() == "GetStreamUriResponse")
    {
        // Get the RTSP path from Message
        rtspPath = getStreamUriResponse.arguments().child("MediaUri").childValues().child("Uri").value().toString();
        return rtspPath;
    }
    else
        this->DeviceError("Error while getting RTSP path");
    return rtspPath;
}

// Getting resolution of the camera
QSize OnvifDevice::getResolution() {
    KDSoapMessage mediaProfile =  this->getMediaProfile(mediaPoint);
    QSize Resolution;
    if(mediaProfile.name() == "GetProfilesResponse")
    {
        //get total available profile names
        QList<KDSoapValue> profiles = mediaProfile.arguments();

        foreach(KDSoapValue profile, profiles){
            // get non empty profile
            if(profile.childValues().size() > 2){
                // Get the Resolution Width and Height
                Resolution.rwidth() =  profile.childValues().child("VideoEncoderConfiguration").childValues().child("Resolution").childValues().child("Width") .value().toInt();
                Resolution.rheight() =  profile.childValues().child("VideoEncoderConfiguration").childValues().child("Resolution").childValues().child("Height") .value().toInt();
                return Resolution;
            }
        }
    }
    else
        this->DeviceError("Error: While getting Resolution");
    return Resolution;
}

// getting the Frame Rate
int OnvifDevice::getFps() {
    KDSoapMessage mediaProfile =  this->getMediaProfile(mediaPoint);

    if(mediaProfile.name() == "GetProfilesResponse")
    {
        //get total available profile names
        QList<KDSoapValue> profiles = mediaProfile.arguments();

        foreach(KDSoapValue profile, profiles){
            // get non empty profile
            if(profile.childValues().size() > 2){
                // get Frame rate limit
                int FrameRate = profile.childValues().child("VideoEncoderConfiguration").childValues().child("RateControl").childValues().child("FrameRateLimit") .value().toInt();
                return FrameRate;
            }
        }
    }
    else
        this->DeviceError("Error: While getting Frame Rate Limit");
    return -1;
}

// getting Bitrate limit
int OnvifDevice::getBitrate() {
    KDSoapMessage mediaProfile =  this->getMediaProfile(mediaPoint);

    if(mediaProfile.name() == "GetProfilesResponse")
    {
        //get total available profile names
        QList<KDSoapValue> profiles = mediaProfile.arguments();

        foreach(KDSoapValue profile, profiles){
            // get non empty profile
            if(profile.childValues().size() > 2){
                // getting bitrate from message
                int Bitrate  = profile.childValues().child("VideoEncoderConfiguration").childValues().child("RateControl").childValues().child("BitrateLimit") .value().toInt();
                return Bitrate;
            }
        }
    }
    else
        this->DeviceError("Error: While getting Bitrate Limit");
    return -1;
}

// setting the resolution of camera
QSize OnvifDevice::setResolution(QSize resolution) {

    // saving resolution value in default settings and these settings will be set by
    // setVideoEncoderConfiguration method

    VideoEncoderSettings["RWidth"] = resolution.rwidth();
    VideoEncoderSettings["RHeight"] = resolution.rheight();
    QSize resValue = resolution;

    // setting resolution of camera with value in default settings
    KDSoapMessage setVideoEncoderConfigurationResponse =  this->setVideoEncoderConfiguration(mediaPoint);

    if(setVideoEncoderConfigurationResponse.name()!= "SetVideoEncoderConfigurationResponse") {
        this->DeviceError("Error: While setting Resolution");
        return QSize(-1, -1);
    } else {
        qDebug()<<"\n....setting new Resoltuion to  width x height  :  " << VideoEncoderSettings["RWidth"].toInt() << " x " << VideoEncoderSettings["RHeight"].toInt();
        return resValue;
    }
}

// setting Frame rate of camera
int OnvifDevice::setFps(int Fps) {
    // Temperarily storing the frame rate in default settings
    VideoEncoderSettings["FRate"] = Fps;

    KDSoapMessage setVideoEncoderConfigurationResponse =  this->setVideoEncoderConfiguration(mediaPoint);
    if(setVideoEncoderConfigurationResponse.name()!= "SetVideoEncoderConfigurationResponse") {
        this->DeviceError("Error: While setting Frame Rate Limit");
        return -1;
    } else {
        qDebug()<<"\n....setting new Frame rate to  :  " << VideoEncoderSettings["FRate"].toInt();
        return VideoEncoderSettings["FRate"].toInt();
    }
}

// setting the bitrate of Camera
int OnvifDevice::setBitrate(int bitrate) {

    // Temporarily store the bitrate in default settings
    VideoEncoderSettings["BRate"] = bitrate;

    KDSoapMessage setVideoEncoderConfigurationResponse =  this->setVideoEncoderConfiguration(mediaPoint);
    if(setVideoEncoderConfigurationResponse.name()!= "SetVideoEncoderConfigurationResponse") {
        this->DeviceError("Error: While setting Bitrate Limit");
        return -1;
    } else {
        qDebug()<<"\n....setting new Bit rate to  :  "<< VideoEncoderSettings["BRate"].toInt();
        return VideoEncoderSettings["BRate"].toInt();
    }
}

// getting the Device Info
QHash <QString, QVariant> OnvifDevice::getDeviceInformation() {
    DeviceBindingService dbs;
    QHash <QString, QVariant> DeviceInfo;
    dbs.setEndPoint(DeviceServiceEndpoint);
    dbs.setSoapVersion(KDSoapClientInterface::SOAP1_2);
    dbs.clientInterface()->setHeader("Security",this->getAuthenticationHeader(Username, Password));
    TDS__GetDeviceInformationResponse deviceInfo = dbs.getDeviceInformation();

    DeviceInfo["Manufacturer"] = deviceInfo.manufacturer();
    DeviceInfo["Model"] = deviceInfo.model();
    DeviceInfo["Serial"] = deviceInfo.serialNumber();
    DeviceInfo["HardwareID"] = deviceInfo.hardwareId();
    DeviceInfo["Firmware"] =  deviceInfo.firmwareVersion();
    return DeviceInfo;
}

// getting various Device Service End points for connecting to device
void OnvifDevice::ConnectToDeviceService() {
    DeviceBindingService dbs;
    dbs.setEndPoint(DeviceServiceEndpoint);
    dbs.setSoapVersion(KDSoapClientInterface::SOAP1_2);

    TDS__GetCapabilities getCapability;
    getCapability.category().append(TT__CapabilityCategory::All);
    TDS__GetCapabilitiesResponse cap = dbs.getCapabilities(getCapability);

    // Device Media point
    this->mediaPoint = cap.capabilities().media().xAddr();

    // Device Service endpoint
    this->DeviceServiceEndpoint = cap.capabilities().device().xAddr();

    // Device event point
    this->EventPint = cap.capabilities().events().xAddr();
}

// getting Authentication Header with Username and password
KDSoapMessage OnvifDevice::getAuthenticationHeader(QString username, QString password) {
    KDSoapMessage HeaderMessage;
    Utils utils;
    QString wsse("http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd");
    QString utility("http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd");

    // Authentication header with user credential
    KDSoapValue header;
    KDSoapValue SecurityToken  = this->getToken(QLatin1String("Security"), QVariant(), true, wsse);
    header.childValues().append(SecurityToken);

    // timestamp token
    KDSoapValue TimeStampToken = this->getToken(QLatin1String("Timestamp"), QVariant(), true, utility);

    // Timestamp token attribute
    KDSoapValue TS_attr = this->getToken(QLatin1String("Id"), QVariant("Time"), true, utility);
    TimeStampToken.childValues().attributes().append(TS_attr);

    // child token for Timestamp token
    QString currentDateTime = QDateTime::currentDateTime().toUTC().toString("yyyy-MM-ddTHH:mm:ssZ");
    KDSoapValue CreatedToken = this->getToken(QLatin1String("Created"), currentDateTime, true, utility);
    TimeStampToken.childValues().append(CreatedToken);

    QDateTime ExpDateTime = QDateTime::currentDateTime().toUTC();
    QDateTime ExpireDateTime  = ExpDateTime.addSecs(10);
    KDSoapValue ExpireToken = this->getToken(QLatin1String("Expires"), ExpireDateTime.toString("yyyy-MM-ddTHH:mm:ssZ"), true, utility);

    // appending Timestamp token
    TimeStampToken.childValues().append(ExpireToken);
    header.childValues().child(QLatin1String("Security")).childValues().append(TimeStampToken);

    // username token
    KDSoapValue UsernameToken = this->getToken(QLatin1String("UsernameToken"),QVariant(), true, wsse);

    // ucsername Token attribute
    KDSoapValue UNT_attr = this->getToken(QLatin1String("Id"), QVariant("User"), true, utility);
    UsernameToken.childValues().attributes().append(UNT_attr);

    // child token for the Username token
    KDSoapValue UserToken = this->getToken(QLatin1String("Username"), QVariant(username), true, wsse);
    UsernameToken.childValues().append(UserToken);

    //setting the digest for the password
    QString nonce = utils.getNonceEncoded();
    QString password_digest = utils.getPasswordDigest(nonce, currentDateTime, password);

    KDSoapValue PasswordToken = this->getToken(QLatin1String("Password"), QVariant(password_digest), true, wsse);

    // attribute for Password token
    KDSoapValue PASS_attr = this->getToken(QLatin1String("Type"), QVariant("http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest"), true, "");
    PasswordToken.childValues().attributes().append(PASS_attr);
    UsernameToken.childValues().append(PasswordToken);

    // getting nonce value
    QVariant nonceValue(nonce);
    KDSoapValue NonceToken = this->getToken(QLatin1String("Nonce"), nonceValue, true, wsse);
    UsernameToken.childValues().append(NonceToken);

    KDSoapValue UCreatedToken = this->getToken(QLatin1String("Created"), currentDateTime, true, utility);
    UsernameToken.childValues().append(UCreatedToken);
    header.childValues().child(QLatin1String("Security")).childValues().append(UsernameToken);//Appending Username token
    HeaderMessage = header;

    return HeaderMessage;
}

// This function will get the media profile
KDSoapMessage OnvifDevice::getMediaProfile(QString MediaServiceEndpoint) {
    DeviceBindingService dbs;
    dbs.setEndPoint(MediaServiceEndpoint);
    dbs.setSoapVersion(KDSoapClientInterface::SOAP1_2);

    dbs.clientInterface()->setHeader("Security",this->getAuthenticationHeader(Username, Password));
    KDSoapMessage getProfilesMessage;
    KDSoapValue parameter("GetProfiles", QVariant(), QString::fromLatin1("http://www.onvif.org/ver10/media/wsdl"), QString::fromLatin1("GetProfiles"));// converter_complextype.cpp:471

    const QString action = QString::fromLatin1("http://www.onvif.org/ver10/media/wsdl/GetProfiles");
    KDSoapValue _valueGetProfiles(parameter);
    _valueGetProfiles.setNamespaceUri(QString::fromLatin1("http://www.onvif.org/ver10/media/wsdl"));
    getProfilesMessage = _valueGetProfiles;// elementargumentserializer.cpp:122
    KDSoapMessage getProfilesResponse = dbs.clientInterface()->call(QLatin1String("GetProfiles"), getProfilesMessage, action);
    return getProfilesResponse;
}

// Getting stream URI
KDSoapMessage OnvifDevice::getStreamURI(QString MediaServiceEndpoint) {
    DeviceBindingService dbs;
    dbs.setEndPoint(MediaServiceEndpoint);
    dbs.setSoapVersion(KDSoapClientInterface::SOAP1_2);
    //set the header
    dbs.clientInterface()->setHeader("Security",this->getAuthenticationHeader(Username, Password));

    KDSoapMessage getStreamURIMessage;
    const QString action = QString::fromLatin1("http://www.onvif.org/ver10/media/wsdl/GetStreamUri");
    KDSoapValue parameter("GetStreamUri", QVariant(), QString::fromLatin1("http://www.onvif.org/ver10/media/wsdl"), QString::fromLatin1("GetStreamUri"));// converter_complextype.cpp:471

    // Stream setup token
    KDSoapValue StreamSetup = this->getToken(QLatin1String("StreamSetup"), QVariant(), true, "http://www.onvif.org/ver10/media/wsdl");
    parameter.childValues().append(StreamSetup);

    KDSoapValue StreamToken = this->getToken(QLatin1String("Stream"), QVariant("RTPS-Unicast"), true, "http://www.onvif.org/ver10/media/wsdl");

    // Appending Stream Token
    parameter.childValues().child("StreamSetup").childValues().append(StreamToken);
    KDSoapValue TransportToken = this->getToken(QLatin1String("Transport"), QVariant(), true, "http://www.onvif.org/ver10/media/wsdl");
    parameter.childValues().child("StreamSetup").childValues().append(TransportToken);

    // Protocol Token With Protocol Name
    KDSoapValue ProtocolToken = this->getToken(QLatin1String("Protocol"), QVariant("UDP"), true, "http://www.onvif.org/ver10/media/wsdl");
    parameter.childValues().child("StreamSetup").childValues().child("Transport").childValues().append(ProtocolToken);

    // Profile token with Profile Name
    KDSoapValue ProfileToken = this->getToken(QLatin1String("ProfileToken"), QVariant(VideoEncoderSettings["MediaProfileTokenValue"]), true, "http://www.onvif.org/ver10/media/wsdl");
    parameter.childValues().append(ProfileToken);

    KDSoapValue _valueGetStreamURI(parameter);
    _valueGetStreamURI.setNamespaceUri(QString::fromLatin1("http://www.onvif.org/ver10/media/wsdl"));

    getStreamURIMessage = _valueGetStreamURI;// elementargumentserializer.cpp:122
    KDSoapMessage streamUriResponse = dbs.clientInterface()->call(QLatin1String("GetStreamUri"), getStreamURIMessage, action);

    return streamUriResponse;
}

// get video encoder configuration
KDSoapMessage OnvifDevice::getVideoEncoderConfiguration(QString MediaServiceEndpoint) {
    DeviceBindingService dbs;
    dbs.setEndPoint(MediaServiceEndpoint);
    dbs.setSoapVersion(KDSoapClientInterface::SOAP1_2);

    dbs.clientInterface()->setHeader("Security",this->getAuthenticationHeader(Username, Password));

    KDSoapMessage getVideoEncoderConfiguration;
    KDSoapValue parameter("GetVideoEncoderConfigurations", QVariant(), QString::fromLatin1("http://www.onvif.org/ver10/media/wsdl"), QString::fromLatin1("GetVideoEncoderConfigurations"));// converter_complextype.cpp:471
    const QString action = QString::fromLatin1("http://www.onvif.org/ver10/media/wsdl/GetVideoEncoderConfigurations");

    KDSoapValue _valueGetVideoEncoderConfigurations(parameter);
    _valueGetVideoEncoderConfigurations.setNamespaceUri(QString::fromLatin1("http://www.onvif.org/ver10/media/wsdl"));
    getVideoEncoderConfiguration = _valueGetVideoEncoderConfigurations;// elementargumentserializer.cpp:122
    KDSoapMessage videoEncoderConfigurations = dbs.clientInterface()->call(QLatin1String("GetVideoEncoderConfigurations"),getVideoEncoderConfiguration, action);
    return videoEncoderConfigurations;
}

// set video Encoder configuration
KDSoapMessage OnvifDevice::setVideoEncoderConfiguration(QString MediaServiceEndpoint) {
    DeviceBindingService dbs;
    dbs.setEndPoint(MediaServiceEndpoint);
    dbs.setSoapVersion(KDSoapClientInterface::SOAP1_2);

    dbs.clientInterface()->setHeader("Security",this->getAuthenticationHeader(Username, Password));
    KDSoapMessage videoEncoderConfigurationSettings;
    const QString action = QString::fromLatin1("http://www.onvif.org/ver10/media/wsdl/SetVideoEncoderConfiguration");
    KDSoapValue parameter("SetVideoEncoderConfiguration", QVariant(), QString::fromLatin1("http://www.onvif.org/ver10/media/wsdl"), QString::fromLatin1("SetVideoEncoderConfiguration"));// converter_complextype.cpp:471

    // Configuration token
    KDSoapValue Configuration = this->getToken(QLatin1String("Configuration"), QVariant(), true, "http://www.onvif.org/ver10/media/wsdl");
    parameter.childValues().append(Configuration);

    KDSoapValue ConTokAttr(QLatin1String("token"),QVariant(VideoEncoderSettings["EncoderConfigTokenValue"]));
    parameter.childValues().child("Configuration").childValues().attributes().append(ConTokAttr);
    KDSoapValue NameToken = this->getToken(QLatin1String("Name"), QVariant(VideoEncoderSettings["EncoderConfigTokenValue"]), true, "http://www.onvif.org/ver10/schema");//VideoEncoderSettings["VideoEncoderConfigurationMain"])

    // Appending Name Token
    parameter.childValues().child("Configuration").childValues().append(NameToken);
    KDSoapValue UseCountToken = this->getToken(QLatin1String("UseCount"), QVariant(1), true, "http://www.onvif.org/ver10/schema");
    parameter.childValues().child("Configuration").childValues().append(UseCountToken);

    // Encoding Token  //values JPEG, H264
    KDSoapValue EncodingToken = this->getToken(QLatin1String("Encoding"), QVariant("H264"), true, "http://www.onvif.org/ver10/schema");
    parameter.childValues().child("Configuration").childValues().append(EncodingToken);

    // Resolution Token
    KDSoapValue ResolutionToken = this->getToken(QLatin1String("Resolution"), QVariant(), true, "http://www.onvif.org/ver10/schema");
    parameter.childValues().child("Configuration").childValues().append(ResolutionToken);

    // Resolution Width Token
    KDSoapValue ResolutionWidth = this->getToken(QLatin1String("Width"), QVariant(VideoEncoderSettings["RWidth"]), true, "http://www.onvif.org/ver10/schema");
    parameter.childValues().child("Configuration").childValues().child("Resolution").childValues().append(ResolutionWidth);

    // Resolution Height Token
    KDSoapValue ResolutionHeight = this->getToken(QLatin1String("Height"), QVariant(VideoEncoderSettings["RHeight"]), true, "http://www.onvif.org/ver10/schema");
    parameter.childValues().child("Configuration").childValues().child("Resolution").childValues().append(ResolutionHeight);

    // Quality Token
    KDSoapValue QualityToken = this->getToken(QLatin1String("Quality"), QVariant(7), true, "http://www.onvif.org/ver10/schema");
    parameter.childValues().child("Configuration").childValues().append(QualityToken);

    // RateControl Token
    KDSoapValue RateControlToken = this->getToken(QLatin1String("RateControl"), QVariant(), true, "http://www.onvif.org/ver10/schema");
    parameter.childValues().child("Configuration").childValues().append(RateControlToken);

    // Frame Rate Limit Token
    KDSoapValue FrameRateLimit = this->getToken(QLatin1String("FrameRateLimit"), QVariant(VideoEncoderSettings["FRate"]), true, "http://www.onvif.org/ver10/schema");
    parameter.childValues().child("Configuration").childValues().child("RateControl").childValues().append(FrameRateLimit);

    // Encoding Interval Token
    KDSoapValue EncodingInterval = this->getToken(QLatin1String("EncodingInterval"), QVariant(1), true, "http://www.onvif.org/ver10/schema");
    parameter.childValues().child("Configuration").childValues().child("RateControl").childValues().append(EncodingInterval);

    // Bitrate Limit Token
    KDSoapValue BitrateLimit = this->getToken(QLatin1String("BitrateLimit"), QVariant(VideoEncoderSettings["BRate"]), true, "http://www.onvif.org/ver10/schema");
    parameter.childValues().child("Configuration").childValues().child("RateControl").childValues().append(BitrateLimit);

    // Multicast Token
    KDSoapValue MulticastToken = this->getToken(QLatin1String("Multicast"), QVariant(), true, "http://www.onvif.org/ver10/schema");
    parameter.childValues().child("Configuration").childValues().append(MulticastToken);

    // Address Token
    KDSoapValue Address = this->getToken(QLatin1String("Address"), QVariant(), true, "http://www.onvif.org/ver10/schema");
    parameter.childValues().child("Configuration").childValues().child("Multicast").childValues().append(Address);

    // Address Type Token
    KDSoapValue AddressType = this->getToken(QLatin1String("Type"), QVariant("IPv4"), true, "http://www.onvif.org/ver10/schema");
    parameter.childValues().child("Configuration").childValues().child("Multicast").childValues().child("Address").childValues().append(AddressType);

    // IP Address Token
    KDSoapValue IPv4Address = this->getToken(QLatin1String("IPv4Address"), QVariant("238.255.255.255"), true, "http://www.onvif.org/ver10/schema");
    parameter.childValues().child("Configuration").childValues().child("Multicast").childValues().child("Address").childValues().append(IPv4Address);

    // Port Token
    KDSoapValue Port = this->getToken(QLatin1String("Port"), QVariant(25320), true, "http://www.onvif.org/ver10/schema");
    parameter.childValues().child("Configuration").childValues().child("Multicast").childValues().append(Port);

    // TTL Token
    KDSoapValue TTL = this->getToken(QLatin1String("TTL"), QVariant(60), true, "http://www.onvif.org/ver10/media/wsdl");
    parameter.childValues().child("Configuration").childValues().child("Multicast").childValues().append(TTL);

    // AutoStart Token
    KDSoapValue AutoStart = this->getToken(QLatin1String("AutoStart"), QVariant(false), true, "http://www.onvif.org/ver10/schema");
    parameter.childValues().child("Configuration").childValues().child("Multicast").childValues().append(AutoStart);

    // SessionTimeout Token
    KDSoapValue SessionTimeout = this->getToken(QLatin1String("SessionTimeout"), QVariant("PT0S"), true, "http://www.onvif.org/ver10/schema");
    parameter.childValues().child("Configuration").childValues().append(SessionTimeout);

    // ForcePersistence token
    KDSoapValue ForcePersistence = this->getToken(QLatin1String("ForcePersistence"), QVariant(true), true, "http://www.onvif.org/ver10/media/wsdl");
    parameter.childValues().append(ForcePersistence);

    KDSoapValue _valueSetVideoEncoderSettings(parameter);
    _valueSetVideoEncoderSettings.setNamespaceUri(QString::fromLatin1("http://www.onvif.org/ver10/media/wsdl"));

    videoEncoderConfigurationSettings = _valueSetVideoEncoderSettings;// elementargumentserializer.cpp:122
    KDSoapMessage setVideoEncoderSettingResponse = dbs.clientInterface()->call(QLatin1String("SetVideoEncoderConfiguration"),videoEncoderConfigurationSettings, action);

    return setVideoEncoderSettingResponse;
}

// get video Encoder Configuration options
KDSoapMessage OnvifDevice::getVideoEncoderConfigurationOptions(QString MediaServiceEndpoint) {
    DeviceBindingService dbs;
    dbs.setEndPoint(MediaServiceEndpoint);
    dbs.setSoapVersion(KDSoapClientInterface::SOAP1_2);

    dbs.clientInterface()->setHeader("Security",this->getAuthenticationHeader(Username, Password));
    KDSoapMessage getVideoEncoderConfigurationOptions;
    KDSoapValue parameter("GetVideoEncoderConfigurationOptions", QVariant(), QString::fromLatin1("http://www.onvif.org/ver10/media/wsdl"), QString::fromLatin1("GetVideoEncoderConfigurationOptions"));// converter_complextype.cpp:471
    const QString action = QString::fromLatin1("http://www.onvif.org/ver10/media/wsdl/GetVideoEncoderConfigurationOptions");

    KDSoapValue _valueGetVideoEncoderConfigurationOptions(parameter);
    _valueGetVideoEncoderConfigurationOptions.setNamespaceUri(QString::fromLatin1("http://www.onvif.org/ver10/media/wsdl"));
    getVideoEncoderConfigurationOptions = _valueGetVideoEncoderConfigurationOptions;// elementargumentserializer.cpp:122
    KDSoapMessage GetVideoEncoderConfigurationOptions = dbs.clientInterface()->call(QLatin1String("GetVideoEncoderConfigurationOptions"),getVideoEncoderConfigurationOptions, action);
    return GetVideoEncoderConfigurationOptions;
}

// getting the available resolutions for the given encoder name e.g. JPEG, H264
QList <QSize> OnvifDevice::getAvailableResolution(QString videoEncoderName) {
    QList <QSize> RValuesList;
    KDSoapMessage configurationOptions = this->getVideoEncoderConfigurationOptions(this->mediaPoint);
    QList <KDSoapValue> ResolutionList =  configurationOptions.arguments().child("Options").childValues().child(videoEncoderName).childValues();

    // check is there any resolutions is available
    if (ResolutionList.size() >= 1) {
        QSize ResValue;
        foreach(KDSoapValue resolution, ResolutionList)
        {
            // get the value of the resolution from the list
            if(resolution.name() == "ResolutionsAvailable"){
                ResValue.rwidth() = resolution.childValues().child("Width").value().toInt();
                ResValue.rheight() =  resolution.childValues().child("Height").value().toInt();
                RValuesList.append(ResValue);
            }
        }
    }
    else
        qDebug()<<" No Resolution options available";
    return RValuesList;
}
