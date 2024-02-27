#ifndef INDEX_H
#define INDEX_H

#define COMPRESS


#include <fstream>
#include <iostream>
#include <queue>
#include <sstream>
#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <vector>
#include <filesystem>

#include "comp_dict.h"
#include "textprocess.h"
#include "postinglist.h"
#include "corpus.h"

template<typename Container = PostingList, typename MapContainer = std::map<std::string, Container>>
class Index {
private:
    std::unique_ptr<MapContainer> dictionary;

    static bool compare(const std::tuple<std::string, std::set<int>, int>& a, const std::tuple<std::string, std::set<int>, int>& b) {
        return std::get<0>(a) > std::get<0>(b);
    }
#ifndef COMPRESS
    void writeToFile(const std::string& filename) const {
        std::ofstream indexFile(filename);
        if (!indexFile.is_open()) {
            std::cerr << "Error: Could not open file for writing." << std::endl;
            return;
        }

        for (const auto& entry : *dictionary) {
            const auto& key = entry.first;
            const auto& postingList = entry.second;

            indexFile << key << ":";
            for (size_t i = 0; i < postingList.size(); ++i) {
                if (i > 0) {
                    indexFile << ",";
                }
                indexFile << postingList.getDocIds().at(i);
            }

            indexFile << std::endl;
        }

        indexFile.close();
    }
#endif

    void writeToFile(const std::string& filename, std::unique_ptr<std::map<std::string, std::set<int>>>& index) const {
        std::ofstream indexFile(filename);
        if (!indexFile.is_open()) {
            std::cerr << "Error: Could not open file for writing." << std::endl;
            return;
        }

        for (const auto& entry : *index) {
            const auto& key = entry.first;
            const auto& postingList = entry.second;

            indexFile << key << ":";
            bool firstElement = true;

            for (const int& element : postingList) {
                if (!firstElement) {
                    indexFile << ",";
                }
                indexFile << element;
                firstElement = false;
            }

            indexFile << std::endl;
        }

        indexFile.close();
    }


    void loadFromFile(const std::string& filename) const {
        std::ifstream indexFile(filename);
        if (!indexFile.is_open()) {
            std::cerr << "Error: Could not open file for loading." << std::endl;
            return;
        }

        std::string line;
        while (std::getline(indexFile, line)) {
            std::istringstream iss(line);
            std::string key;
            std::vector<int> docIds;

            std::getline(iss, key, ':');
            std::string docIdStr;
            while (std::getline(iss, docIdStr, ',')) {
                docIds.push_back(std::stoi(docIdStr));
            }
            dictionary->insert(std::make_pair(key, Container{docIds.begin(), docIds.end()}));
        }

        indexFile.close();
    }

public:

    //Index construction procedure: SPIMI inverting + partial dictionary merging -> compressed index
    Index(Corpus& corpus, size_t maxIndexSize) {
        std::string filePath("index_file.txt"); 
        dictionary = std::make_unique<MapContainer>();
        std::ifstream file(filePath);
        if (file.is_open()) {
            file.close();
            loadFromFile(filePath);
        }else{
            size_t currentIndexSize = 0;
            std::pair<int, std::string> pair;
            int file_num = 0;
            std::string filename = "temp_index-" + std::to_string(file_num);
            auto temp_dictionary = std::make_unique<std::map<std::string, std::set<int>>>();

            while (corpus >> pair) {
                if (textProcessing(pair.second)) continue;

                if (currentIndexSize + pair.second.size() * sizeof(int) > maxIndexSize) {
                    // Save the current index to disk and reset it.
                    writeToFile(filename, temp_dictionary);
                    currentIndexSize = 0;
                    filename.clear();
                    temp_dictionary->clear();
                    file_num++;
                    filename = "temp_index-" + std::to_string(file_num);
                }

                auto [it, inserted] = temp_dictionary->emplace(pair.second, std::set<int>{pair.first});
                if (!inserted) it->second.insert(pair.first);

                currentIndexSize += pair.second.size() * sizeof(int);
            }

            // Save the final index to disk.
            if (temp_dictionary->size() > 0) {
                writeToFile(filename, temp_dictionary);
                temp_dictionary->clear();
            }

            //start the merging procedure
            
            std::priority_queue<std::tuple<std::string, std::set<int>, int>, std::vector<std::tuple<std::string, std::set<int>, int>>, decltype(&compare)> pq(compare);
            std::vector<std::ifstream> fileStreamVec;
            //fileStreamVec.resize(file_num+1);
            
            for (int i = 0; i <= file_num; i++) {
                std::string filename = "temp_index-" + std::to_string(i);
                std::ifstream iftream(filename);
                if (iftream.is_open()) {
                    fileStreamVec.push_back(std::move(iftream));
                } else {
                    
                }
            }

            int idx = 1;
            for (auto& file_stream : fileStreamVec) {
                std::string line;
                std::getline(file_stream, line);
                std::istringstream iss(line);
                std::string key;
                std::set<int> docIds;
                std::getline(iss, key, ':');
                std::string docIdStr;
                while (std::getline(iss, docIdStr, ',')) {
                    docIds.insert(std::stoi(docIdStr));
                }
                pq.push(std::make_tuple(key, docIds, idx));
                idx++;
            }

            // Process the triplets in order
            while (!pq.empty()) {

                auto merge_tuple = pq.top();
                pq.pop();

                int fnum = std::get<2>(merge_tuple);
                // Read the next triplet from the corresponding file
                if(!fileStreamVec[fnum-1].eof()){

                    std::string line;
                    std::getline(fileStreamVec[fnum-1], line);
                    std::istringstream iss(line);
                    std::string key;
                    std::set<int> docIds;
                    std::getline(iss, key, ':');
                    std::string docIdStr;
                    while (std::getline(iss, docIdStr, ',')) {
                        docIds.insert(std::stoi(docIdStr));
                    }
                    pq.push(std::make_tuple(key, docIds, fnum));
                }

                while(std::get<0>(pq.top())==std::get<0>(merge_tuple) && !pq.empty()){

                    auto top = pq.top();
                    pq.pop();
                    int last_file_num = std::get<2>(top);
                    if(!fileStreamVec[last_file_num-1].eof()){
                    // Read the next triplet from the corresponding file
                        std::string line;
                        std::getline(fileStreamVec[last_file_num-1], line);
                        std::istringstream iss(line);
                        std::string key;
                        std::set<int> docIds;
                        std::getline(iss, key, ':');
                        std::string docIdStr;
                        while (std::getline(iss, docIdStr, ',')) {
                            docIds.insert(std::stoi(docIdStr));
                        }
                        pq.push(std::make_tuple(key, docIds, last_file_num));
                    }

                    auto merged_set = std::get<1>(merge_tuple);
                    merged_set.insert(std::get<1>(top).begin(), std::get<1>(top).end());
                    merge_tuple = std::make_tuple(std::get<0>(merge_tuple), merged_set, std::get<2>(merge_tuple));

                }
                dictionary->insert(std::make_pair(std::get<0>(merge_tuple), Container{std::get<1>(merge_tuple).begin(), std::get<1>(merge_tuple).end()}));
            }

            for (int i = 0; i <= file_num; i++) {
                std::string filename = "temp_index-" + std::to_string(i);
                // Add the file streams to the map
                fileStreamVec[i].close();
                std::filesystem::remove(filename);
            }
#ifndef COMPRESS
            writeToFile(filePath);
#endif
        }
    }

    const Container& getPostingList(const std::string& term) const {
        try {
        return dictionary->at(term);
        } catch (const KeyNotFoundError& e) {
        }
    }
};

#endif // INDEX_H

