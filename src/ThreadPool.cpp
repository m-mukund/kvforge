#include "ThreadPool.h"
#include "KVStore.h"
#include "StorageManager.h"
#include "ProtocolParser.h"
#include <iostream>
#include <iterator>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <vector>

ThreadPool::ThreadPool(size_t num_threads, KVStore& db, StorageManager& storage): db(db), stop(false), storage(storage){

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
    std::string connection_buffer;  // Useful incase TCP packet gets fragmented

    while (true) {
        ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
        if (bytes_read <= 0) {
            break; 
        }
        buffer[bytes_read] = '\0';

        connection_buffer+=buffer;

        auto [tokens, consumed] = parse_resp(connection_buffer);

        if(consumed==0)
            continue;

        connection_buffer.erase(0, consumed);

        std::string response;
        

        if(tokens[0]=="SET"&&tokens.size()>=3){
            db.set(tokens[1], tokens[2]);
            storage.append_command(tokens);
            response="+OK\r\n";
        }
        else if(tokens[0]=="GET"&&tokens.size()>=2){
            std::optional<std::string> val1=db.get(tokens[1]);
            if(!val1)
                response="$-1\r\n";
            else
                response="$"+std::to_string(val1->length())+"\r\n"+*val1+"\r\n";
        }
        else if (tokens[0]=="DEL"&&tokens.size()>=2) {
            bool del_status=db.del(tokens[1]);
            if(del_status){
                storage.append_command(tokens);
                response=":1\r\n";
            }
            else
                response=":0\r\n";
        }
        else {
            response="-ERR Invalid Command/ Parameters\r\n";
        }

        

        write(client_socket, response.c_str(), response.size());
    }
    close(client_socket);
}