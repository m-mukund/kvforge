#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include <string>
#include <vector>
#include <fstream>
#include <mutex>

class StorageManager{
private:
    std::ofstream aof_file;
    std::mutex file_mutex;

public:
    StorageManager(const std::string& filename);
    ~StorageManager();

    void append_command(const std::vector<std::string>& tokens);

};

#endif