#ifndef SOCKET_H
#define SOCKET_H

#include <QObject>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef WIN32
#include <WinSock2.h>
#include <WS2ipdef.h>
#define socklen_t int
#define SHUT_RDWR SD_BOTH

#else
#define closesocket(s) close(s)
#endif

static int SetSocketTimeout(int s, int timeout) {
    timeval tv;
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;
    return setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

static int ReuseAddress(int s) {
    int reuseAddr = 1;
    return setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr));
}

#endif // SOCKET_H
