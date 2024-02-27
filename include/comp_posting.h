// compressedpostinglist.h
#ifndef COMP_POSTING_H
#define COMP_POSTING_H


#include <cstdint>
#include <algorithm>
#include <iostream>
#include <ostream>
#include <vector>

class compressedPostingList {
private:
    std::vector<uint8_t> docIds;

    uint64_t decode_unsigned_varint( const uint8_t *const data, int &decoded_bytes ) const
    {
        int i = 0;
        uint64_t decoded_value = 0;
        int shift_amount = 0;

        do 
        {
            decoded_value |= (uint64_t)(data[i] & 0x7F) << shift_amount;     
            shift_amount += 7;
        } while ( (data[i++] & 0x80) != 0 );

        decoded_bytes = i;
        return decoded_value;
    }

    // Encode an unsigned 64-bit varint.  Returns number of encoded bytes.
    // 'buffer' must have room for up to 10 bytes.
    int encode_unsigned_varint(uint8_t *const buffer, uint64_t value) const
    {
        int encoded = 0;

        do
        {
            uint8_t next_byte = value & 0x7F;
            value >>= 7;

            if (value)
                next_byte |= 0x80;

            buffer[encoded++] = next_byte;

        } while (value);


        return encoded;
    }

    std::vector<uint8_t> encodeDocumentIds(const std::vector<int>& docIds){
        std::vector<uint8_t> encodedDocIds;
        std::vector<uint8_t> tempBuffer;
        tempBuffer.resize(8);

        if (!docIds.empty())
        {
            tempBuffer.resize(encode_unsigned_varint(tempBuffer.data(), docIds[0]));
            encodedDocIds.insert(encodedDocIds.end(), tempBuffer.begin(), tempBuffer.end());

            for (std::size_t i = 1; i < docIds.size(); ++i)
            {
                uint64_t diff = (uint64_t)docIds[i] - docIds[i - 1];
                tempBuffer.clear();
                tempBuffer.resize(8);
                tempBuffer.resize(encode_unsigned_varint(tempBuffer.data(), diff));
                encodedDocIds.insert(encodedDocIds.end(), tempBuffer.begin(), tempBuffer.end());
            }
        }
        return encodedDocIds;
    }


public:

    template<typename Iterator>
    compressedPostingList(Iterator begin, Iterator end)
        : docIds(encodeDocumentIds(std::vector<int>(begin, end))) {}

    compressedPostingList(const std::vector<int>& docIds)
        : docIds(encodeDocumentIds(docIds)) {}

    compressedPostingList(const std::vector<uint8_t>& docIds)
        : docIds(docIds) {}

    compressedPostingList() = default;


    void print() const {
        for (const auto& value : docIds) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }

    const std::vector<uint8_t>& getDocIds() const {
        return docIds;
    }

    std::vector<uint8_t>& getDocIds() {
        return docIds;
    }

    std::vector<int> getDecodedIds() {
        std::vector<int> decodedDocIds;
        int decoded_bytes = 0;
        uint64_t decoded_value = 0;
        int previous_value = 0;

        for (std::size_t i = 0; i < docIds.size(); ) {
            decoded_value = decode_unsigned_varint(docIds.data() + i, decoded_bytes);
            decodedDocIds.push_back(previous_value + decoded_value);
            previous_value = decodedDocIds.back();
            i += decoded_bytes;
        }

        return decodedDocIds;
    }

    std::vector<int> getDecodedIds() const {
        std::vector<int> decodedDocIds;
        int decoded_bytes = 0;
        uint64_t decoded_value = 0;
        int previous_value = 0;

        for (std::size_t i = 0; i < docIds.size(); ) {
            decoded_value = decode_unsigned_varint(docIds.data() + i, decoded_bytes);
            decodedDocIds.push_back(previous_value + decoded_value);
            previous_value = decodedDocIds.back();
            i += decoded_bytes;
        }

        return decodedDocIds;
    }

    compressedPostingList OR(const compressedPostingList other) const {
        auto ls = getDecodedIds();
        auto rs = other.getDecodedIds();
        std::vector<int> buffer;
        std::set_union(ls.begin(), ls.end(), rs.begin(), rs.end(),
                       std::inserter(buffer, buffer.begin()));
        compressedPostingList result(buffer);
        return result;
    }

    compressedPostingList AND(const compressedPostingList other) const {
        auto ls = getDecodedIds();
        auto rs = other.getDecodedIds();
        std::vector<int> buffer;
        std::set_intersection(ls.begin(), ls.end(), rs.begin(), rs.end(),
                       std::inserter(buffer, buffer.begin()));
        compressedPostingList result(buffer);
        return result;
    }

    compressedPostingList ANDNOT(const compressedPostingList other) const {
        auto ls = getDecodedIds();
        auto rs = other.getDecodedIds();
        std::vector<int> buffer;
        std::set_difference(ls.begin(), ls.end(), rs.begin(), rs.end(),
                       std::inserter(buffer, buffer.begin()));
        compressedPostingList result(buffer);
        return result;
    }

/*
    compressedPostingList AND(const compressedPostingList other) const{
    std::vector<uint8_t> intersectedDocIds;
        // get the smaller docIds vector
        const auto& smallerDocIds = (this->size() < other.size()) ? this->getDocIds() : other.getDocIds();
        const auto& largerDocIds = (smallerDocIds == this->getDocIds()) ? other.getDocIds() : this->getDocIds();

        int decoded_bytes = 0;
        for (size_t i = 0; i < smallerDocIds.size(); ) {
            uint64_t currentDocId = decode_unsigned_varint(smallerDocIds.data(), decoded_bytes);
            decoded_bytes += 7; // move the decoded_bytes pointer to the next varint

            // check if the currentDocId exists in largerDocIds
            uint64_t largerDocIdsBeginPos = 0;
            while (largerDocIdsBeginPos < largerDocIds.size()) {
                uint64_t diff = 0;
                uint64_t nextLargerDocId = decode_unsigned_varint(largerDocIds.data() + largerDocIdsBeginPos, decoded_bytes);
                decoded_bytes += 7; // move the decoded_bytes pointer to the next varint

                // if next largerDocId is smaller than currentDocId, continue to the next largerDocId
                if (nextLargerDocId < currentDocId) {
                    largerDocIdsBeginPos += decoded_bytes;
                    continue;
                }

                std::vector<uint8_t> tempBuffer(8);

                // if next largerDocId is equal to currentDocId, add it to the result and move to the next largerDocId
                if (nextLargerDocId == currentDocId) {
                    intersectedDocIds.push_back(encode_unsigned_varint(tempBuffer.data(), currentDocId));
                    break;
                }

                // calculate the diff between next largerDocId and currentDocId, if it's smaller or equal to the diff, continue to the next largerDocId
                diff = nextLargerDocId - currentDocId;
                if (diff <= std::numeric_limits<uint64_t>::max() - nextLargerDocId) {
                    currentDocId += diff;
                    continue;
                }
            }

            // if currentDocId doesn't exist in largerDocIds, move to the next varint
            if (largerDocIdsBeginPos >= largerDocIds.size()) {
                i += decoded_bytes;
                continue;
            }
        }

        return intersectedDocIds;
    }*/

    std::size_t size() const {
        return docIds.size();
    }

    friend std::ostream& operator<<(std::ostream& os, const compressedPostingList& pl) {
        auto decodedIds = pl.getDecodedIds();
        for (const auto& value : decodedIds) {
            os << value << " ";
        }
        return os;
    }


};

#endif // COMP_POSTING_H
