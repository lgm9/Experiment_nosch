#pragma once
#include<pthread.h>
#include <sys/socket.h>
#include <queue>
#include "payload.h"
#include "rocksdb/db.h"

class Worker {
    int ID;
    int sockfd;
    char* buf, *key, *value;
    Payload* pl;
    pthread_t w_thread;
    std::queue<Payload *> *Q;
    rocksdb::DB* db;
    std::mutex* Q_lock;
    int i;
    inline int parselen();

    public:
        pthread_mutex_t *lock;
        pthread_cond_t *cv;
        Worker(int, int, pthread_mutex_t*, pthread_cond_t*, rocksdb::DB*, std::mutex*, std::queue<Payload *>*);
        int work();
        int init();
        ~Worker() {
            free(buf);
        }
};

static void* work_wrapper(void *);