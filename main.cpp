#include <cstdio>
#include <time.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <queue>
#include <mutex>
#include "worker.h"
#include "rocksdb/db.h"
#include "payload.h"
#define BUF_SIZE 128

rocksdb::DB* db;
rocksdb::Options options;
rocksdb::Status status;

struct sockaddr_in serv_addr, cli_addr;
socklen_t addrlen = sizeof(cli_addr);
int server_fd, client_fd, rclen;
char* mainbuf;

Worker **workers;
pthread_mutex_t *worker_lock;
pthread_cond_t* worker_cond;

int num_workers;

std::queue<Payload *> MainQ;
std::mutex MainQ_lock;

int init_socket() {
    mainbuf = (char *)malloc(BUF_SIZE * sizeof(char));

    server_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if(server_fd <= 0) {
        printf("Failed to initalize socket\n");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(4000);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(-1 == bind(server_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) {
        printf("bind error\n");
        return 1;
    }
    
    printf("Successfully initalized socket\n");
    return 0;
}

void init_db() {
    options.create_if_missing = true;
    status = rocksdb::DB::Open(options, "/tmp/testdb_nosch", &db);
    assert(status.ok());
    printf("Initialized DB\n");
}

void init_worker() {
    workers = (Worker **)malloc(num_workers * sizeof(Worker *));
    worker_lock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * num_workers);
    worker_cond = (pthread_cond_t *)malloc(sizeof(pthread_cond_t) * num_workers);
    
    for(int i = 0 ; i < num_workers ; i++) {
        pthread_mutex_init(&worker_lock[i], NULL);
        pthread_cond_init(&worker_cond[i], NULL);
        workers[i] = new Worker(i, server_fd, &worker_lock[i], &worker_cond[i], db, &MainQ_lock, &MainQ);
        workers[i] -> init();
    }
    printf("Initialized workers\n");
}

void socket_loop() {
    while(1) {
        rclen = recvfrom(server_fd, mainbuf, BUF_SIZE, 0, (struct sockaddr*)&cli_addr, &addrlen);
        MainQ.push(new Payload(rclen, mainbuf, cli_addr));
    }
}

int main(int argc, char *argv[]) {
    if(argc < 2) {
        printf("The number of workers should be given\n");
        return 0;
    }
    num_workers = atoi(argv[1]);

    if(init_socket()) {
        return 0;
    }
    
    init_db();
    init_worker();
    socket_loop();

    close(server_fd);

    return 0;
}