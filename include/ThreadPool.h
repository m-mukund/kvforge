#pragma once

#include "KVStore.h"

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

class ThreadPool{
public:
    explicit ThreadPool(size_t num_threads, KVStore& db);

    ~ThreadPool();

    void enqueue(int client_socket);

private:

    KVStore& db;

    std::vector<std::thread> workers;
    std::queue<int> tasks;

    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop; //flag to tell threads to shut down

    void worker_loop();

    void handle_client(int client_socket);

};