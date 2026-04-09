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
#include <vector>

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

std::vector<std::string> ThreadPool::parse_resp(const std::string& buffer) {
    std::vector<std::string> tokens;
    size_t cursor = 0;

    // Sanity check
    if (buffer.empty() || buffer[cursor] != '*') return tokens; 

    // Find array length
    size_t first_crlf = buffer.find("\r\n", cursor);
    if (first_crlf == std::string::npos) return tokens; 

    int num_args = std::stoi(buffer.substr(1, first_crlf-1)); 
 
    cursor = first_crlf + 2;

    for (int i = 0; i < num_args; ++i) {
        if (cursor >= buffer.size() || buffer[cursor] != '$') break;

        size_t length_crlf = buffer.find("\r\n", cursor);
        if (length_crlf == std::string::npos) break;

        int str_len = std::stoi(buffer.substr(cursor+1, length_crlf-(cursor+1)));
        
        cursor = length_crlf + 2;

        // Safety Check before substr
        if (cursor + str_len > buffer.size())
            return {};

        tokens.push_back(buffer.substr(cursor, str_len)); 

        cursor = cursor + str_len + 2;
    }

    return tokens;
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

        std::vector<std::string> tokens = parse_resp(connection_buffer);

        if(tokens.empty())
            continue;

        connection_buffer.clear();

        std::string response;
        

        if(tokens[0]=="SET"&&tokens.size()>=3){
            db.set(tokens[1], tokens[2]);
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
            if(del_status)
                response=":1\r\n";
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