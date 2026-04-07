#include "KVStore.h"
#include <cstddef>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>

KVStore::KVStore(size_t capacity):capacity(capacity){}

void KVStore::set(const std::string& key, const std::string& value){
    std::unique_lock<std::shared_mutex> lock(rw_mutex);

    auto it=store.find(key);
    if(it!=store.end()){
        lru_list.splice(lru_list.begin(), lru_list, it->second);
        lru_list.front().second=value;
    }
    else{
        if(store.size()==capacity){
            std::string ru=lru_list.back().first;
            lru_list.pop_back();
            store.erase(ru);
        }
        lru_list.insert(lru_list.begin(), {key, value});
        store[key]=lru_list.begin();
    }
}

std::optional<std::string> KVStore::get(const std::string& key) {
    std::unique_lock<std::shared_mutex> lock(rw_mutex);
    
    auto it=store.find(key);
    if(it!=store.end()){
        lru_list.splice(lru_list.begin(), lru_list, it->second);
        return lru_list.begin()->second;
    }
        
    return std::nullopt;
}

bool KVStore::del(const std::string& key){

    std::unique_lock<std::shared_mutex> lock(rw_mutex);

    auto it=store.find(key);
    if(it!=store.end()){
        lru_list.erase(it->second);
        store.erase(it);
        return true;
    }
    return false;
}