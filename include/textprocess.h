#ifndef TEXTPROCESS_H
#define TEXTPROCESS_H


#include <algorithm>
#include <codecvt>
#include <locale>
#include <string>
#include "../OleanderStemmingLibrary/src/english_stem.h"

const std::string stopWords[] = {
    "ourselves", "hers", "between", "yourself", "but", "again", "there", "about", "once", "during", "out", "very", "having",
    "with", "they", "own", "an", "be", "some", "for", "do", "its", "yours", "such", "into", "of", "most", "itself", "other", "off", "is", "s",
    "am", "or", "who", "as", "from", "him", "each", "the", "themselves", "until", "below", "are", "we", "these", "your", "his", "through", "don",
    "nor", "me", "were", "her", "more", "himself", "this", "down", "should", "our", "their", "while", "above", "both", "up", "to", "ours", "had",
    "she", "all", "no", "when", "at", "any", "before", "them", "same", "and", "been", "have", "in", "will", "on", "does", "yourselves", "then", 
    "that", "because", "what", "over", "why", "so", "can", "did", "not", "now", "under", "he", "you", "herself", "has", "just", "where", "too",
    "only", "myself", "which", "those", "i", "after", "few", "whom", "t", "being", "if", "theirs", "my", "against", "a", "by", "doing", "it", 
    "how", "further", "was", "here", "than"
};

const int sizeStop = sizeof(stopWords)/sizeof(stopWords[0]);

inline bool isStopWord(const std::string& word){
    for(int i=0;i<sizeStop;i++){
        if(word==stopWords[i])
            return true;
    }
    return false;
}

inline int textProcessing(std::string& word) {
    std::transform(word.begin(), word.end(), word.begin(), [](unsigned char c){ return std::tolower(c); });
    word.erase(std::remove_if(word.begin(), word.end(), [](char c) { return (!std::isalpha(c) && !std::isalnum(c)); }), word.end());
    //std::set<char> punctuation_set { '`', '.', ',', '-', ':', '\'', '\"', '?', '!', '/', ';', '(', ')', '{', '}', '[', ']', '&', '|', '~', '_', '#', '@', '<', '>', '$', '%', '*', '\t', '”', '“' };
    //word.erase(std::remove_if(word.begin(), word.end(),[&punctuation_set](char c) {return punctuation_set.count(c) > 0; }), word.end());
    if (isStopWord(word)) 
        return 1;
    stemming::english_stem<> StemEnglish;
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring wstr = converter.from_bytes(word);
    StemEnglish(wstr);
    word = converter.to_bytes(wstr);
    word.erase(std::remove_if(word.begin(), word.end(), [](char c) { return (!std::isalpha(c) && !std::isalnum(c));; }), word.end());
    if(word.empty()) return 1;
    return 0;
}

#endif // TEXTPROCESS_H