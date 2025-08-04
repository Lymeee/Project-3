#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <chrono>

#include "AdjacencyList.h"

// Dear ImGui
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

struct Steam_Properties {
    int appID;
    std::string name;
    std::string reviewText;
    int score;
    int votes;
};

std::vector<Steam_Properties> loadReviews(
    const std::string& fileName,
    std::map<std::string,int>& uniqueTitles)
{
    std::vector<Steam_Properties> out;
    std::ifstream file(fileName);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << fileName << "\n";
        return out;
    }

    std::string line;
    std::getline(file, line); 
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        Steam_Properties r;
        std::string field;

        // app ID
        if (!std::getline(ss, field, ',')) continue;
        try { r.appID = std::stoi(field); } catch(...) { continue; }

        // name/title
        if (!std::getline(ss, field, ',')) continue;
        if (!field.empty() && field.front()=='"') {
            std::string part;
            while (field.back()!='"' && std::getline(ss, part, ',')) {
                field += "," + part;
            }
            if (field.front()=='"' && field.back()=='"')
                field = field.substr(1, field.size()-2);
        }
        r.name = field;
        if (r.name.empty()) continue;

        // review
        if (!std::getline(ss, field, ',')) continue;
        if (!field.empty() && field.front()=='"') {
            std::string part;
            while (field.back()!='"' && std::getline(ss, part, ',')) {
                field += "," + part;
            }
            if (field.front()=='"' && field.back()=='"')
                field = field.substr(1, field.size()-2);
        }
        r.reviewText = field;

        // score
        if (!std::getline(ss, field, ',')) continue;
        try { r.score = std::stoi(field); } catch(...) { continue; }

        // votes
        if (!std::getline(ss, field, ',')) continue;
        try { r.votes = std::stoi(field); } catch(...) { continue; }

        out.push_back(r);
        uniqueTitles.emplace(r.name, r.appID);
    }
    return out;
}

void buildGraphFromReviews(
    const std::vector<Steam_Properties>& reviews,
    AdjacencyList& graph)
{
    // Group reviews by game ID
    std::unordered_map<int,std::vector<std::string>> byGame;
    for (auto& r : reviews)
        byGame[r.appID].push_back(r.reviewText);

    // Connect every pair of reviews for each game
    for (auto& kv : byGame) {
        auto& texts = kv.second;
        for (size_t i = 0; i < texts.size(); ++i)
            for (size_t j = i+1; j < texts.size(); ++j)
                graph.makeEdge(texts[i], texts[j]);
    }
}

// Calculates all statistics ONCE
std::map<std::string, std::vector<int>> titleScores; // collects game title and vector of scores
std::map<std::string, float> averageScores; // the average score = sum of all scores divided by total # of scores per game

void calculateStats(
    const std::vector<Steam_Properties>& reviews, 
    std::map<std::string, std::vector<int>>& titleScores,
    std::map<std::string, float>& averageScores)
{
    for (auto& r : reviews) {
        // game title not found, create empty vector in map
        if(titleScores.find(r.name) == titleScores.end()) {
            titleScores.insert({r.name, std::vector<int>{}});
        }
        // push back regardless
        titleScores[r.name].push_back(r.score);
    }
    
    // map created 

    // averages scores for each title
    for (auto& ts : titleScores) {
        // std::cout << ts.first << " has " << ts.second.size() << " scores." << std::endl; // test

        float sum = 0.0; // score resets per title
        for (auto& e : ts.second) {
            sum += e;
        }

        // game title for average scores map not found
        if (averageScores.find(ts.first) == averageScores.end()) {
            averageScores[ts.first] = sum/ts.second.size();
        }
    }

    // // test print average scores
    // for (auto& as : averageScores) {
    //     std::cout << as.first << " has an average score of " << as.second << std::endl;
    // }
}

int main() {
    // Initialize GLFW and create window
    if (!glfwInit()) return 1;
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(1200,900,"Steam Review Visualizer",NULL,NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Initialize ImGui context and backends
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window,true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Show loading screen 
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Loading");
        ImGui::Text("Loading reviews, please wait...");
        ImGui::End();
        ImGui::Render();
        int w,h; glfwGetFramebufferSize(window,&w,&h);
        glViewport(0,0,w,h);
        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Load reviews from CSV and collect unique game titles
    std::map<std::string,int> uniqueTitles;
    auto allReviews = loadReviews("steam_sample.csv", uniqueTitles);

    calculateStats(allReviews, titleScores, averageScores);

    // Prepare game names for filter dropdown
    std::vector<std::string> gameNames{"All Games"};
    for (auto& kv : uniqueTitles)
        if (!kv.first.empty())
            gameNames.push_back(kv.first);

    // Prepare filtered reviews and graph
    std::vector<Steam_Properties> filtered = allReviews;
    AdjacencyList graph;
    buildGraphFromReviews(filtered, graph);

    // UI state variables
    static int   gameFilterIdx   = 0;
    static int   scoreFilterIdx  = 0;
    static int   minReviewsFilter  = 0;
    static int   minAverageScore = 0;
    static char  keyword[128]    = "";
    static int   searchType      = 0;
    std::vector<std::string> resultNodes;
    long long elapsed_us = 0;
    static bool searched = false;
    static bool running = true;

    std::string lastSearchStart;

 
    static long long bfs_us = 0, dfs_us = 0;
    static size_t bfs_nodes = 0, dfs_nodes = 0;

    // Get glfw frame size for window resize
    struct glfwFrame {
        int width, height;
    };

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        glfwFrame outsideFrame;
        glfwGetFramebufferSize(window, &outsideFrame.width, &outsideFrame.height);
        ImGui::SetNextWindowSize(ImVec2(outsideFrame.width,outsideFrame.height), ImGuiCond_Always);

        ImGui::Begin("Steam Review Visualizer", &running, 
            ImGuiWindowFlags_NoCollapse | 
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoTitleBar);

        // Tab bar for UI sections
        if (ImGui::BeginTabBar("MainTabBar")) {
            // Filters tab
            if (ImGui::BeginTabItem("Filters")) {
                // Game filter dropdown
                if (ImGui::BeginCombo("Game Filter", gameNames[gameFilterIdx].c_str())) {
                    for (int i=0; i<(int)gameNames.size(); ++i) {
                        bool sel = (i==gameFilterIdx);
                        if (ImGui::Selectable(gameNames[i].c_str(), sel))
                            gameFilterIdx = i;
                        if (sel) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                // Minimum reviews slider
                ImGui::SliderInt("Min Reviews", &minReviewsFilter, 0, 100); 
                // Recommendation filter
                ImGui::Combo("Recommendation", &scoreFilterIdx,
                             "All\0Recommended\0Not Recommended\0");
                // Apply filters button
                if (ImGui::Button("Apply Filters")) {
                    filtered = allReviews;
                    // Filter by game
                    if (gameFilterIdx>0) {
                        int id = uniqueTitles[gameNames[gameFilterIdx]];
                        filtered.erase(std::remove_if(
                            filtered.begin(), filtered.end(),
                            [&](auto& r){ return r.appID!=id; }),
                        filtered.end());
                    }
                    // Filter by recommendation
                    if (scoreFilterIdx==1) {
                        filtered.erase(std::remove_if(
                            filtered.begin(), filtered.end(),
                            [](auto& r){ return r.score!=1; }),
                        filtered.end());
                    } else if (scoreFilterIdx==2) {
                        filtered.erase(std::remove_if(
                            filtered.begin(), filtered.end(),
                            [](auto& r){ return r.score!=-1; }),
                        filtered.end());
                    }
                    // Filter by minimum votes
                    if (minReviewsFilter>0) {
                        filtered.erase(std::remove_if(
                            filtered.begin(), filtered.end(),
                            [&](auto& r){ return r.votes<minReviewsFilter; }),
                        filtered.end());
                    }
                    // Rebuild graph and clear results
                    graph = AdjacencyList();
                    buildGraphFromReviews(filtered, graph);
                    resultNodes.clear();
                }
                ImGui::EndTabItem();
            }

            // Search tab
            if (ImGui::BeginTabItem("Search")) {
                // Keyword input
                ImGui::InputText("Keyword", keyword, IM_ARRAYSIZE(keyword));
                // Search type  
                ImGui::RadioButton("BFS",&searchType,0); ImGui::SameLine();
                ImGui::RadioButton("DFS",&searchType,1);
                // Run search button
                if (ImGui::Button("Run")) {
                    searched = true;
                    std::string kw(keyword);
                    std::transform(kw.begin(), kw.end(), kw.begin(), ::tolower);
                    std::string start;
                    // Find first review containing keyword
                    for (auto& r : filtered) {
                        std::string low = r.reviewText;
                        std::transform(low.begin(), low.end(), low.begin(), ::tolower);
                        if (low.find(kw)!=std::string::npos) {
                            start = r.reviewText;
                            lastSearchStart = start; 
                            break;
                        }
                    }
                    resultNodes.clear();
                    // Run BFS or DFS if found
                    if (!start.empty()) {
                        auto t0 = std::chrono::high_resolution_clock::now();
                        resultNodes = (searchType==0)
                            ? graph.BFS(start)
                            : graph.DFS(start);
                        auto t1 = std::chrono::high_resolution_clock::now();
                        elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(
                                         t1 - t0).count();

                        // Store BFS/DFS comparison results for the Comparison tab
                        auto t_bfs0 = std::chrono::high_resolution_clock::now();
                        auto bfsResult = graph.BFS(start);
                        auto t_bfs1 = std::chrono::high_resolution_clock::now();
                        bfs_us = std::chrono::duration_cast<std::chrono::microseconds>(t_bfs1 - t_bfs0).count();
                        bfs_nodes = bfsResult.size();

                        auto t_dfs0 = std::chrono::high_resolution_clock::now();
                        auto dfsResult = graph.DFS(start);
                        auto t_dfs1 = std::chrono::high_resolution_clock::now();
                        dfs_us = std::chrono::duration_cast<std::chrono::microseconds>(t_dfs1 - t_dfs0).count();
                        dfs_nodes = dfsResult.size();
                        // -------------------------------------------------------------
                    } else {
                        elapsed_us = 0;
                        bfs_us = dfs_us = 0;
                        bfs_nodes = dfs_nodes = 0;
                    }
                }
                // Show elapsed time
                float ms = elapsed_us * 0.001f;
                ImGui::Text("Elapsed: %.2f ms", ms);
                // Show message if no results
                if (searched && resultNodes.empty()) {
                    ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "No results found.");
                }
                ImGui::EndTabItem();
            }

            // Results tab
            if (ImGui::BeginTabItem("Results")) {
                ImGui::Text("Visited: %zu nodes", resultNodes.size());
                ImGui::Separator();
                // List result nodes
                for (auto& s : resultNodes) {
                    ImGui::Bullet();
                    ImGui::SameLine();
                    ImGui::TextWrapped("%s", s.c_str());
                }
                ImGui::EndTabItem();
            }

            // Statistics tab
            if (ImGui::BeginTabItem("Statistics")) {
                ImGui::TextWrapped("Displaying review statistics for %s", gameNames[gameFilterIdx].c_str());

                // statistics at the top for applied game/all
                if (gameNames[gameFilterIdx] == "All Games") {
                    ImGui::TextWrapped("Total reviews across all games catalogued: %i", allReviews.size());
                }
                else {
                    ImGui::TextWrapped("Total number of reviews: %i", titleScores[gameNames[gameFilterIdx]].size());
                    ImGui::TextWrapped("Average Score: %.3f", averageScores[gameNames[gameFilterIdx]]);
                    if (averageScores[gameNames[gameFilterIdx]] > 0) {
                        ImGui::TextWrapped("Positively rated");
                    }
                    else if (averageScores[gameNames[gameFilterIdx]] < 0) {
                        ImGui::TextWrapped("Negatively rated");
                    }
                    else {
                        ImGui::TextWrapped("Neutrally rated");
                    }
                }
                
                ImGui::Separator();

                ImGui::Spacing();
                ImGui::TextWrapped("Average Scores: ");

                // Minimum average score slider
                ImGui::SliderInt("Min Average Score (%)", &minAverageScore, 0, 100);
                int numGamesFiltered = 0; 
                for (auto& g : averageScores) {
                    if (minAverageScore == 0) {
                        numGamesFiltered = uniqueTitles.size();
                    }
                    else if (g.second >= minAverageScore/100.0) {
                        numGamesFiltered++;
                    }
                }

                // displays filtered number of games matching min average score slider criteria
                ImGui::TextWrapped("Showing %i games", numGamesFiltered);
                ImGui::Separator();

                for (auto& g : averageScores) {
                    if (minAverageScore == 0) {
                        ImGui::Bullet();
                        ImGui::SameLine();
                        ImGui::TextWrapped("%s has an average score of %.3f", g.first.c_str(), g.second);
                    }
                    else if (g.second >= minAverageScore/100.0) {
                        ImGui::Bullet();
                        ImGui::SameLine();
                        ImGui::TextWrapped("%s has an average score of %.3f", g.first.c_str(), g.second);
                    }
                }

                ImGui::EndTabItem();
            }

            // Comparison tab
            if (ImGui::BeginTabItem("Comparison")) {
                ImGui::TextWrapped("BFS vs DFS Comparison");
                ImGui::Separator();

                if (!lastSearchStart.empty() && bfs_nodes > 0 && dfs_nodes > 0) {
                    ImGui::TextWrapped("BFS:  %.2f ms, %zu nodes visited", bfs_us * 0.001f, bfs_nodes);
                    ImGui::TextWrapped("DFS:  %.2f ms, %zu nodes visited", dfs_us * 0.001f, dfs_nodes);

                    float max_nodes = (float)std::max(bfs_nodes, dfs_nodes);
                    ImGui::TextWrapped("Nodes Visited:");
                    ImGui::ProgressBar(bfs_nodes / max_nodes, ImVec2(200, 20), "BFS");
                    ImGui::ProgressBar(dfs_nodes / max_nodes, ImVec2(200, 20), "DFS");
                } else {
                    ImGui::TextWrapped("No data to compare. Run a search first.");
                }

                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::End();
        ImGui::Render();
        // Set viewport and clear screen
        int w,h; glfwGetFramebufferSize(window,&w,&h);
        glViewport(0,0,w,h);
        glClearColor(0.1f,0.1f,0.1f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup ImGui and GLFW
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}