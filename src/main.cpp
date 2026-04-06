#include <iostream>
#include <string>
#include <cstring>
#include <sys/_types/_ssize_t.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "ThreadPool.h"
#include "KVStore.h"

int main() {

    int server_fd=socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd==-1){
        std::cerr<<"Failed Socket Creation\n";
        return 1;
    }

    // Set SO_REUSEADDR to allow immediate port reusage
    int opt=1;
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))<0){
        std::cerr<<"Failed to set SO_REUSEADDR\n";
        close(server_fd);
        return 1;
    }

    // Describe the address
    sockaddr_in address{};
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=INADDR_ANY;
    address.sin_port=htons(8080);

    if(bind(server_fd, (struct sockaddr*)&address, sizeof(address))<0){
        std::cerr<<"Bind failed\n";
        close(server_fd);
        return 1;
    }

    //Queue upto 5 incoming connections
    if(listen(server_fd, 5)<0){
        std::cerr<<"Listen Failed\n";
        close(server_fd);
        return 1;
    }

    std::cout << "Multithreaded KVForge Echo Server listening on port 8080..." << std::endl;

    KVStore db;
    ThreadPool pool(4, db);

    //Leader thread Loop
    while(true){
        sockaddr_in client_address{};
        socklen_t client_len=sizeof(client_address);

        // BLOCKING call. The thread will sleep here until a client connects.
        int client_socket=accept(server_fd, (struct sockaddr*)&client_address, &client_len);

        if(client_socket<0){
            std::cerr<<"Accept failed\n";
            close(server_fd);
            return 1;
        }

        std::cout << "New client accepted. Handing off to worker pool..." << std::endl;
        pool.enqueue(client_socket);
    }

    
    close(server_fd);
    std::cout<<"Server Shutting down\n";

    
    return 0;
}