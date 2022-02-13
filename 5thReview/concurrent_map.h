#pragma once

#include <vector>
#include <map>
#include <mutex>
using namespace std::string_literals;

//Реализация структуры ПодМножества
template <typename Key, typename Value>
struct SubMap {
    std::map<Key, Value> sub_map;
    std::mutex sub_map_guard;
};

//Реализация класса ConcurrentMap
template <typename Key, typename Value>
class ConcurrentMap {
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys");

    struct Access {
        Access(std::mutex& mutex, const Key& key, std::map<Key, Value>& sub_map_ref) : value_guard(mutex),
                                                                                       ref_to_value(sub_map_ref[key]) {}
        std::lock_guard<std::mutex> value_guard;
        Value& ref_to_value;
    };
    ConcurrentMap(size_t bucket_count) : sub_maps(bucket_count) {};

    Access operator[](const Key& key) {
        uint64_t key_ = static_cast<uint64_t>(key) % sub_maps.size();
        {
            std::lock_guard guard(sub_maps[key_].sub_map_guard);
        }
        return {sub_maps[key_].sub_map_guard, key, sub_maps[key_].sub_map};
    };

    std::map<Key, Value> BuildOrdinaryMap() {
        std::map<Key, Value> result_map;
        for (size_t i = 0; i < sub_maps.size(); ++i) {
            std::lock_guard<std::mutex> guard(sub_maps[i].sub_map_guard);
            for (const auto& [key, value] : sub_maps[i].sub_map) {
                result_map[key] = value;
            }
        }
        return result_map;
    };

    auto Erase(const Key& key) {
        uint64_t key_ = static_cast<uint64_t>(key) % sub_maps.size();
        std::lock_guard guard(sub_maps[key_].sub_map_guard);
        return sub_maps[key_].sub_map.erase(key);
    }

private:
    std::vector<SubMap<Key, Value>> sub_maps;
};