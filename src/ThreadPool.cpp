#include "ThreadPool.h"
#include "KVStore.h"
#include <iostream>
#include <iterator>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>

ThreadPool::ThreadPool(size_t num_threads, KVStore& db): db(db), stop(false){

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

        std::string request(buffer);
        std::string response;
        std::istringstream iss(request);

        std::vector<std::string> tokens{std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>()};
        if(tokens.empty())
            continue;

        if(tokens[0]=="SET"&&tokens.size()>=3){
            db.set(tokens[1], tokens[2]);
            response="OK\n";
        }
        else if(tokens[0]=="GET"&&tokens.size()>=2){
            std::optional<std::string> val1=db.get(tokens[1]);
            if(!val1)
                response="(nil)\n";
            else
                response=*val1+"\n";
        }
        else if (tokens[0]=="DEL"&&tokens.size()>=2) {
            bool del_status=db.del(tokens[1]);
            if(del_status)
                response="1\n";
            else
                response="0\n";
        }
        else {
            response="ERROR: Invalid Command/ Parameters\n";
        }

        

        write(client_socket, response.c_str(), response.size());
    }
    close(client_socket);
}