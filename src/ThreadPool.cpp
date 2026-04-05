#include "ThreadPool.h"
#include <iostream>
#include <mutex>
#include <unistd.h>
#include <sys/socket.h>

ThreadPool::ThreadPool(size_t num_threads): stop(false){
    for(int i=0;i<num_threads;i++){
        workers.emplace_back([this]{this->worker_loop();});
    }
}

ThreadPool::~ThreadPool(){

    //notify ALL waiting threads to wake up.
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        stop=true;
    }
    condition.notify_all();
    // Joining all the threads
    for(std::thread &worker:workers){
        if(worker.joinable())
            worker.join();
    }
}

void ThreadPool::enqueue(int client_socket){
    
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        tasks.push(client_socket);
    }
    
    condition.notify_one();
}

void ThreadPool::worker_loop(){
    while(true){
        int client_socket;
        {
            // condition variable to wait() until either 'stop' is true OR the 'tasks' queue is not empty.
            std::unique_lock<std::mutex> lock(queue_mutex);

            condition.wait(lock, [this]{ return !tasks.empty()||stop;});

            if(stop&&tasks.empty())
                return;
            
            client_socket = tasks.front();
            tasks.pop();
        } 

        handle_client(client_socket);
    }
}

void ThreadPool::handle_client(int client_socket) {
    char buffer[1024] = {0};
    while (true) {
        ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
        if (bytes_read <= 0) {
            break; 
        }
        buffer[bytes_read] = '\0';
        write(client_socket, buffer, bytes_read);
    }
    close(client_socket);
}