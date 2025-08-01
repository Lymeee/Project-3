#ifndef ADJACENCYLIST_H
#define ADJACENCYLIST_H

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

using namespace std;

class AdjacencyList {
    private:
    unordered_map<string, vector<string>> adj_List;

    public:
    void makeEdge(string x, string y);

    vector<string> BFS(string& x);
    vector<string> DFS(string& x);
};



#endif //ADJACENCYLIST_H
