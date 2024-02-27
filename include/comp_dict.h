#ifndef COMP_DICT_H
#define COMP_DICT_H


#include <map>
#include <stdexcept>
#include <vector>
#include "comp_term.h"

class KeyNotFoundError : public std::runtime_error {
public:
    KeyNotFoundError(const std::string& message)
        : std::runtime_error(message) {}
};

template<typename Container = std::vector<int>>
class compressedDictionary {
private: 
    std::map<compressedTerm, std::vector<Container>> block_map;
    int counter = 0;
public:

    void insert(const std::pair<std::string, Container>& key_value_pair) {
        if(counter==0) {
            auto [it, inserted] = block_map.insert(std::make_pair(compressedTerm(key_value_pair.first), std::vector<Container>()));
            it->second.push_back(key_value_pair.second);
            counter=1;
        } else if(counter<50-1){
            auto last_it = block_map.rbegin();
            last_it->first.addTerm(key_value_pair.first);
            last_it->second.push_back(key_value_pair.second);
            counter++;
        } else {
            counter = 0;
        }
    }

    const Container& at(const std::string& key) const {
        auto it = block_map.lower_bound(key);
        if (it != block_map.end() && it != block_map.begin()) {
            it = std::prev(it);
            auto [index, inserted] = it->first.find(key);
            if(inserted)
                return it->second[index];
        }
        throw KeyNotFoundError("Key not found in the map.");
    }


};

#endif // COMP_DICT_H