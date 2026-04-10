#include "StorageManager.h"
#include <fstream>
#include <ios>
#include <iostream>
#include <string>

StorageManager::StorageManager(const std::string& filename) {
    aof_file.open(filename, std::ios::app);
}

StorageManager::~StorageManager() {
    aof_file.close();
}

void StorageManager::append_command(const std::vector<std::string>& tokens) {
    if (tokens.empty()) return;

    std::string resp_string = "";

    resp_string+="*";
    resp_string+=std::to_string(tokens.size());
    resp_string+="\r\n";
    
    for (const std::string& token : tokens) {
        resp_string+="$";
        resp_string+=std::to_string(token.size());
        resp_string+="\r\n";
        
        resp_string+=token;
        resp_string+="\r\n";
    }

    {
        std::lock_guard<std::mutex> lock(file_mutex);
        
        aof_file<<resp_string;
        aof_file.flush();
    }
}