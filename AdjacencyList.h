#ifndef ADJACENCYLIST_H
#define ADJACENCYLIST_H

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

class AdjacencyList {
private:
    std::unordered_map<std::string, std::vector<std::string>> adj_List;

public:
    void makeEdge(std::string x, std::string y);

    std::vector<std::string> BFS(std::string& x);
    std::vector<std::string> DFS(std::string& x);
};

#endif // ADJACENCYLIST_H
