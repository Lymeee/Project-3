// main.cpp
// Steam Review Comfort Visualizer

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

using namespace std;

struct Steam_Properties {
    string name;
    float score;
    int recommended;
};

// Data loading & filtering
// - loadReviews(filePath): read reviews into vector
// - filterByYear(reviews, year)
// - filterByGenre(reviews, genre)
// - filterByScore(reviews, minScore, maxScore)

vector<Steam_Properties> loadReviews(string fileName) {
    ifstream file(fileName);
    vector<Steam_Properties> result;
    string name;
    float score;
    int recommended;
    
    while (file >> name >> score >> recommended) {
        result.push_back({name, score, recommended});
    }

    //Might need to change based on properties and how it is sorted

    return result;
}

// Algorithms & data representation
// - Represent each game as a node in a graph
// - Connect nodes if games share genre or reviewers
// - BFS(start): count nodes, time it
// - DFS(start): count nodes, time it

// Menu & user interaction
// - showMenu(): print options to console
// - getChoice(): read user’s choice
// - promptFilters(): ask for year, genre, score range
// - promptSearchType(): ask BFS or DFS
// - displayStats(reviews): show review count, average score
// - displayGraphResults(nodesFound, elapsedTime)

// Program flow & integration
// - main loop:
//     load data
//     apply filters
//     choose and run BFS/DFS
//     view stats and graph results
//     exit

int main() {
    bool loaded = false;
    // vector<Review> reviews, filtered;
    // Graph gameGraph;

    while (true) {
        std::cout << "\nSteam Review Comfort Visualizer\n";
        std::cout << "1) Load data\n";
        std::cout << "2) Apply filters\n";
        std::cout << "3) Run BFS or DFS\n";
        std::cout << "4) View stats\n";
        std::cout << "5) Exit\n";
        int choice;
        std::cin >> choice;

        if (choice == 1) {
            // loadReviews("path/to/data");
            loaded = true;
            std::cout << "Data loaded\n";
        }
        else if (choice == 2 && loaded) {
            // promptFilters();
            // filtered = filterBy...();
            std::cout << "Filters applied\n";
        }
        else if (choice == 3 && loaded) {
            // int t = promptSearchType(); // 1 = BFS, 2 = DFS
            // if (t == 1) runBFS(...);
            // else runDFS(...);
            std::cout << "Search complete\n";
        }
        else if (choice == 4 && loaded) {
            // displayStats(filtered);
            // displayGraphResults(...);
        }
        else if (choice == 5) {
            break;
        }
        else {
            if (!loaded) std::cout << "Please load data first\n";
            else std::cout << "Invalid option\n";
        }
    }

    return 0;
}
