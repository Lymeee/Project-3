#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include "AdjacencyList.h"

std::vector<std::string> AdjacencyList::BFS(std::string& x) {
    std::vector<std::string> output;
    std::unordered_set<std::string> visit;
    std::queue<std::string> q;

    visit.insert(x);
    q.push(x);

    while (!q.empty()) {
        std::string cur = q.front();
        q.pop();
        output.push_back(cur);

        // Add more here 
    }

    return output;
}

std::vector<std::string> AdjacencyList::DFS(std::string& x) {
    std::vector<std::string> output;
    std::unordered_set<std::string> visit;
    std::queue<std::string> q;

    visit.insert(x);
    q.push(x);

    while (!q.empty()) {
        std::string cur = q.front();
        q.pop();
        output.push_back(cur);

        // Add more here
    }

    return output;
}
