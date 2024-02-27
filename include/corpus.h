// corpus.h
#ifndef CORPUS_H
#define CORPUS_H


#include <fstream>
#include <iostream>

class Corpus {
private:
    std::ifstream descriptionsFile;
    int currentDocId;
    std::string currentToken;

public:
    Corpus(const std::string& filename) : currentDocId(0), descriptionsFile(filename) {
      if (!descriptionsFile.is_open())  std::cerr << "Error opening file" << std::endl;
      descriptionsFile >> currentDocId;
    }

    void reset() {
        descriptionsFile.clear();
        descriptionsFile.seekg(0, std::ios::beg);
        currentDocId = 0;
    }

    bool operator>>(std::pair<int, std::string>& docIdToken) {
        if (descriptionsFile.is_open()) {
            char next_char = descriptionsFile.peek();
            if (next_char == '\n') {
                descriptionsFile.ignore(); // Ignore the newline character.
                descriptionsFile >> currentDocId;
                descriptionsFile >> currentToken;
            } else {
              descriptionsFile >> currentToken;
            }
            docIdToken.first = currentDocId;
            docIdToken.second = currentToken;
        
        }
        return !descriptionsFile.eof();; // Return the object reference
    }

    ~Corpus() {
        if (descriptionsFile.is_open()) {
            descriptionsFile.close();
        }
    }
};

#endif // CORPUS_H