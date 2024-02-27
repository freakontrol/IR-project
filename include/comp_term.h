#ifndef COMP_TERM_H
#define COMP_TERM_H

#include <memory>
#include <string>

class compressedTerm {
private:
    static std::shared_ptr<std::string> dict_as_a_String;
    static const int block_size = 50;
    size_t block;

    size_t common_prefix_length(const std::string& str1, const std::string& str2) const {
        size_t len = std::min(str1.length(), str2.length());
        for (size_t i = 0; i < len; i++) {
            if (str1[i] != str2[i]) {
                return i;
            }
        }
        return len;
    }

public:
    compressedTerm() = delete;
    compressedTerm(std::string term) {
        block = dict_as_a_String->length();
        dict_as_a_String->push_back(static_cast<size_t>(term.length()));
        dict_as_a_String->append(term);
    }

    static std::shared_ptr<std::string> getCompressedDictionary() {
        return dict_as_a_String;
    }

    bool addTerm(const std::string& term) const{
        int actual_terms_num = 0;
        std::string actual_term = this->firstString();
        std::string block_as_a_string = dict_as_a_String->substr(block);
        size_t relative_position = actual_term.length()+1;
        while (relative_position < block_as_a_string.length()){
            size_t term_len = static_cast<size_t>(block_as_a_string[relative_position]);
            size_t prefix_len = static_cast<size_t>(block_as_a_string[relative_position+1]);
            std::string postfix = block_as_a_string.substr(relative_position+2, term_len-prefix_len);
            std::string prefix = actual_term.substr(0, prefix_len);
            actual_term = prefix + postfix;
            relative_position += 2 + postfix.length();
            actual_terms_num++;
        }
        if(actual_terms_num<block_size-1){
            size_t term_prefix_len =  common_prefix_length(actual_term, term);
            dict_as_a_String->push_back(static_cast<char>(term.length()));
            dict_as_a_String->push_back(static_cast<char>(term_prefix_len));
            dict_as_a_String->append(term.substr(term_prefix_len));
            return true;
        } else return false;
    }

    std::pair<size_t, bool> find(const std::string term) const{
        int actual_terms_num = 0;
        std::string actual_term = this->firstString();
        if(term==actual_term) return std::make_pair(actual_terms_num, true);
        std::string block_as_a_string = dict_as_a_String->substr(block);
        size_t relative_position = actual_term.length()+1;
        while (relative_position < block_as_a_string.length()){
            size_t term_len = static_cast<size_t>(block_as_a_string[relative_position]);
            size_t prefix_len = static_cast<size_t>(block_as_a_string[relative_position+1]);
            std::string postfix = block_as_a_string.substr(relative_position+2, term_len-prefix_len);
            std::string prefix = actual_term.substr(0, prefix_len);
            actual_term = prefix + postfix;
            relative_position += 2 + postfix.length();
            actual_terms_num++;
            if(term==actual_term) return std::make_pair(actual_terms_num, true);
        }
         return std::make_pair(actual_terms_num, false);
    }

    // Destructor
    ~compressedTerm() {}

    std::string firstString() {
        return dict_as_a_String->substr(block+1, static_cast<size_t>(dict_as_a_String->at(block)));
    }

    std::string firstString() const {
        return dict_as_a_String->substr(block+1, static_cast<size_t>(dict_as_a_String->at(block)));
    }

    bool operator<(const compressedTerm& other) const {
        return this->firstString() < other.firstString();
    }

    bool operator<(compressedTerm& other)  {
        return this->firstString() < other.firstString();
    }
};

extern std::shared_ptr<std::string> dict_as_a_String;

#endif // COMP_TERM_H