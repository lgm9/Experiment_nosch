#include <cstdlib>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "payload.h"

Payload::Payload(int l, char* inbuf, struct sockaddr_in inaddr) {
    addr = inaddr;
    buf = (char *)malloc(l + 1);
    len = l;
    for(int i = 0 ; i < l ; i++) buf[i] = inbuf[i];
    for(int i = 0 ; i < l ; i++) inbuf[i] = 0;
    buf[l] = 0;
}