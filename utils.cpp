/**
 * The utils class have the fuctions that are required to generate the password digest.
 *
 * The functions such as getEncodedNonce, getPasswordDigest.
 */

#include <QTime>
#include <QCryptographicHash>

#include "utils.h"

Utils::Utils(){}
Utils::~Utils(){}

// getting  the random nonce
int Utils::getNonce() {
    QTime now = QTime::currentTime();
    uint no = (uint)now.msec();
    qsrand(no);
    int ret = qrand();
    return ret;
}

// getting the base64 encoded nonce
QString Utils::getNonceEncoded() {
    //generated nonce no.
    QString gen_nonce = QString::number(this->getNonce());
    QByteArray nonce_str(gen_nonce.toStdString().c_str());

    //get base64 of nonce
    QString ret(nonce_str.toBase64());

    return ret;
}

// getting the password digest
QString Utils::getPasswordDigest(QString nonce, QString date_time, QString pass_word) {

    // base64 encoded nonce
    QString base64_enc_nonce(nonce);

    //nonce bytes
    QByteArray nonce_bytes = QByteArray::fromBase64 (base64_enc_nonce.toUtf8());
    QByteArray byteArray_for_hash(nonce_bytes);

    //date time bytes
    QString utc_datetime (date_time);
    QByteArray datetime (utc_datetime.toUtf8());
    byteArray_for_hash.append (datetime);

    //password bytes
    QByteArray password(pass_word.toUtf8());
    byteArray_for_hash.append(password);

    //getting SHA1 and Base64
    QByteArray password_sha1 = QCryptographicHash::hash(byteArray_for_hash, QCryptographicHash::Sha1);
    QByteArray password_Digest(password_sha1.toBase64());

    return QString::fromLocal8Bit(password_Digest);
}
