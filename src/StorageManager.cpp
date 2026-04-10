#include "StorageManager.h"
#include "KVStore.h"
#include "ProtocolParser.h"
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
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

void StorageManager::load_aof(KVStore& db){
    std::ifstream file("appendonly.aof", std::ios::binary);

    if (!file.is_open())
        return;

    std::string buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    std::cout << "Loading AOF file into memory...\n";

    while (!buffer.empty()) {
        auto [tokens, consumed]=parse_resp(buffer);

        if(consumed==0)
            break;

        if (tokens[0] == "SET" && tokens.size() >= 3) {
            db.set(tokens[1], tokens[2]);
        } 
        else if (tokens[0] == "DEL" && tokens.size() >= 2) {
            db.del(tokens[1]);
        }

        buffer.erase(0, consumed);
    }

    std::cout << "AOF successfully loaded.\n";
}