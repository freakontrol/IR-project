// external libraries
#include <bits/stdc++.h>
#include <chrono>


// custom classes
#include "../include/corpus.h"
#include "../include/postinglist.h"
#include "../include/comp_posting.h"
#include "../include/index.h"
#include "../include/query.h"


// initialize the dictionary as a string
std::shared_ptr<std::string> compressedTerm::dict_as_a_String(new std::string());

int main() {

    Corpus corpus("MovieSummaries/plot_summaries.txt");
    // specify the maximum memory used during index generation

#ifdef COMPRESS
    Index<compressedPostingList, compressedDictionary<compressedPostingList>> compressedIndex(corpus, 21000000);
#else
    Index<> index(corpus, 50000000);
#endif

    std::cout << "Enter a query (or type 'Q' to quit): ";
    std::string query;

    // Continue asking for queries until the user types 'Q'
    while (std::getline(std::cin, query) && query != "Q") {

        if (query.empty()) {
            continue;
        }

#ifdef COMPRESS
        Query<compressedPostingList, compressedDictionary<compressedPostingList>> userQuery(query, compressedIndex);
#else
        Query<> userQuery(query, index);
#endif
        auto start = std::chrono::high_resolution_clock::now();
        auto result = userQuery.parseAndEvaluate();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "Time to answer the query: " << duration.count() << " microseconds" << std::endl;

        std::cout << result << '\n';

        std::cout << "Enter a query (or type 'Q' to quit): ";
    }

    return 0;
}