#pragma once

#include <string>
#include <unordered_map>
#include <shared_mutex>
#include <optional>

class KVStore{
public:

    void set(const std::string& key, const std::string& value);

    bool del(const std::string& key);

    std::optional<std::string> get(const std::string& key) const;

private:
    std::unordered_map<std::string, std::string> store;
    mutable std::shared_mutex rw_mutex;
};