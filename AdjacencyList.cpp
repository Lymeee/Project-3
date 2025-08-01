#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "AdjacencyList.h"

using namespace std;

vector<string> AdjacencyList::BFS(string& x) {
    vector<string> output;
    unordered_set<string> visit;
    queue<string> q;

    visit.insert(x);
    q.push(x);

    while (!q.empty()) {
        string cur = q.front();
        q.pop();
        output.push_back(cur);

        //Add more here
    }
}

vector<string> AdjacencyList::DFS(string& x) {
    vector<string> output;
    unordered_set<string> visit;
    queue<string> q;

    visit.insert(x);
    q.push(x);


    while (!q.empty()) {
        string cur = q.front();
        q.pop();
        output.push_back(cur);

        //Add more here
    }
}
