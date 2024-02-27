// postinglist.h
#ifndef POSTINGLIST_H
#define POSTINGLIST_H


#include <algorithm>
#include <iostream>
#include <ostream>
#include <vector>

class PostingList {
private:
    std::vector<int> docIds;
public:

    PostingList(std::vector<int> docID) : docIds{docID} {}

    PostingList(std::vector<int>&& docIdsin) : docIds(std::move(docIdsin)) {}

    PostingList() : docIds() {}

    template <typename Iterator>
    PostingList(Iterator begin, Iterator end) : docIds(begin, end) {}

    void addDocID(const int& value) {
        docIds.push_back(value);
    }

    void mergeDocIDs(const std::vector<int>& object) {
        std::merge(docIds.begin(), docIds.end(), object.begin(), object.end(), std::back_inserter(docIds));
    }

    void print() const {
        for (const auto& value : docIds) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }

    const std::vector<int>& getDocIds() const {
        return docIds;
    }

    std::vector<int>& getDocIds() {
        return docIds;
    }
    

    PostingList OR(const PostingList other) const {
        PostingList result;
        std::set_union(docIds.begin(), docIds.end(), other.docIds.begin(), other.docIds.end(),
                       std::inserter(result.docIds, result.docIds.begin()));
        return result;
    }

    PostingList AND(const PostingList other) const {
        PostingList result;
        std::set_intersection(docIds.begin(), docIds.end(), other.docIds.begin(), other.docIds.end(),
                               std::inserter(result.docIds, result.docIds.begin()));
        return result;
    }

    PostingList ANDNOT(const PostingList& other) const {
        PostingList result;
        std::set_difference(docIds.begin(), docIds.end(), other.docIds.begin(), other.docIds.end(),
                            std::inserter(result.docIds, result.docIds.begin()));
        return result;
    }

    std::size_t size() const {
        return docIds.size();
    }

    auto begin() {
        return docIds.begin();
    }

    auto end() {
        return docIds.end();
    }

    friend std::ostream& operator<<(std::ostream& os, const PostingList& pl) {
        for (const auto& value : pl.getDocIds()) {
            os << value << " ";
        }
        return os;
    }

};

#endif // POSTINGLIST_H