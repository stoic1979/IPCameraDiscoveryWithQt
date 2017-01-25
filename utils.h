#ifndef UTILS_H
#define UTILS_H

#include <QObject>

class Utils
{
public:
    Utils();
    ~Utils();

    QString getNonceEncoded();
    QString getPasswordDigest(QString nonce, QString dateTime, QString password);

private:
    int getNonce();

};

#endif // UTILS_H
