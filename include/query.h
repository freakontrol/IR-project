#ifndef QUERY_H
#define QUERY_H


#include <string>
#include "index.h"

enum Operator {
    AND,
    OR,
    AND_NOT,
    NONE
};

template<typename Container = PostingList, typename MapContainer = std::map<std::string, Container>>
class Query {
private:
    std::string query;
    Index<Container, MapContainer> &index;

public:
    Query(const std::string &query, Index<Container, MapContainer> &index) : query(query), index(index) {}

    Container parseAndEvaluate() {
        // Parse the query string into a data structure, such as a tree
        // or a vector of string tokens
        std::vector<std::string> tokens = parseQuery(query);

        // Evaluate the parsed query using the provided Index object
        return evaluateQuery(tokens);
    }

private:
    std::vector<std::string> parseQuery(const std::string &query) {
        std::vector<std::string> tokens;
        std::istringstream iss(query);
        std::string token;

        // Tokenize the query string by spaces
        while (iss >> token) {
            tokens.push_back(token);
        }

        return tokens;
    }

    Container evaluateQuery(const std::vector<std::string> &tokens) {

        std::string currentTerm = "";
        Container currentPostings;
        Operator currentOperator = Operator::NONE;

        // Iterate through the tokens
        for (const std::string &token : tokens) {

            if (isTerm(token)) {
                currentTerm = token;
                textProcessing(currentTerm);
                //PostingList postings = index.getPostingList(currentTerm);

                // Perform the necessary logical operations based on the
                // currentOperator
                switch (currentOperator) {
                    case Operator::AND:
                        // Perform an AND operation using the PostingLists
                        currentPostings = currentPostings.AND(index.getPostingList(currentTerm));
                        break;
                    case Operator::OR:
                        // Perform an OR operation using the PostingLists
                        currentPostings = currentPostings.OR(index.getPostingList(currentTerm));
                        break;
                    case Operator::AND_NOT:
                        // Perform an AND_NOT operation using the PostingLists
                        currentPostings = currentPostings.ANDNOT(index.getPostingList(currentTerm));
                        break;
                    case Operator::NONE:
                        currentPostings = index.getPostingList(currentTerm);
                        break;
                }
                
            } else if (isOperator(token)) {
                // If the token is an operator, save it and update the
                // currentOperator
                currentOperator = parseOperator(token);
            }
        }

        // Return the result of the final logical operation
        return currentPostings;
    }

    bool isTerm(const std::string &token) {
        return !isOperator(token);
    }

    bool isOperator(const std::string &token) {
        return (token == "&&" || token == "||" || token == "&&!" || token == "||!");
    }

    Operator parseOperator(const std::string &token) {
        if (token == "&&") {
            return Operator::AND;
        } else if (token == "||") {
            return Operator::OR;
        } else if (token == "&&!") {
            return Operator::AND_NOT;
        } else {
            throw std::runtime_error("Invalid operator: " + token);
        }
    }
};


#endif // QUERY_H