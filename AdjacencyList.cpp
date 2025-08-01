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

        for (auto y: adj_List[cur]) {
            if (visit.find(y) == visit.end()) {
                visit.insert(y);
                q.push(y);
            }
        } 
    }

    return output;
}

std::vector<std::string> AdjacencyList::DFS(std::string& x) {
    std::vector<std::string> output;
    std::unordered_set<std::string> visit;
    std::stack<std::string> s;

    visit.insert(x);
    s.push(x);

    while (!s.empty()) {
        std::string cur = s.top();
        s.pop();
        output.push_back(cur);

        for (auto y: adj_List[cur]) {
            if (visit.find(y) == visit.end()) {
                visit.insert(y);
                s.push(y);
            }
        }
    }

    return output;
}

void AdjacencyList::makeEdge(std::string x, std::string y) {
    adj_List[y].push_back(x);
    adj_List[x].push_back(y);
}
