#include "KVStore.h"
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>

void KVStore::set(const std::string& key, const std::string& value){
    std::unique_lock<std::shared_mutex> lock(rw_mutex);

    store[key]=value;
}

std::optional<std::string> KVStore::get(const std::string& key) const {
    std::shared_lock<std::shared_mutex> lock(rw_mutex);
    
    auto it=store.find(key);
    if(it!=store.end())
        return it->second;
    return std::nullopt;
}

bool KVStore::del(const std::string& key){

    std::unique_lock<std::shared_mutex> lock(rw_mutex);
    return store.erase(key)>0;
}