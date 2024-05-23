#pragma once
#include <sys/socket.h>

class Payload {

    public:
        char* buf;
        int len;
        struct sockaddr_in addr;
        Payload(int, char*, struct sockaddr_in);
        ~Payload(){
            free(buf);
        }
};