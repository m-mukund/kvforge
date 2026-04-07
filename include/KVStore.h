#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <shared_mutex>
#include <optional>
#include <list>
#include <utility>

class KVStore{
public:

    explicit KVStore(size_t capacity=100);

    void set(const std::string& key, const std::string& value);

    bool del(const std::string& key);

    std::optional<std::string> get(const std::string& key);

private:

    size_t capacity;

    mutable std::shared_mutex rw_mutex;

    std::list<std::pair<std::string, std::string>> lru_list;
    std::unordered_map<std::string, std::list<std::pair<std::string, std::string>>::iterator> store;
};