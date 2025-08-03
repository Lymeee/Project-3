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

// --- Data Model + CSV Loader + Graph Builder ---
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
    std::getline(file, line); // skip header
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        Steam_Properties r;
        std::string field;

        if (!std::getline(ss, field, ',')) continue;
        try { r.appID = std::stoi(field); } catch(...) { continue; }

        if (!std::getline(ss, field, ',')) continue;
        r.name = field;
        if (r.name.empty()) continue;

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

        if (!std::getline(ss, field, ',')) continue;
        try { r.score = std::stoi(field); } catch(...) { continue; }

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

int main() {
    // Initialize GLFW and create window
    if (!glfwInit()) return 1;
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(900,700,"Steam Review Visualizer",NULL,NULL);
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
    static int   minVotesFilter  = 0;
    static char  keyword[128]    = "";
    static int   searchType      = 0;
    std::vector<std::string> resultNodes;
    long long elapsed_us = 0;
    static bool searched = false;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Steam Review Visualizer");

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
                // Minimum votes slider
                ImGui::SliderInt("Min Votes", &minVotesFilter, 0, 100);
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
                            [](auto& r){ return r.score!=0; }),
                        filtered.end());
                    }
                    // Filter by minimum votes
                    if (minVotesFilter>0) {
                        filtered.erase(std::remove_if(
                            filtered.begin(), filtered.end(),
                            [&](auto& r){ return r.votes<minVotesFilter; }),
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
                    } else {
                        elapsed_us = 0;
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
                for (auto& s : resultNodes)
                    ImGui::BulletText("%s", s.c_str());
                ImGui::EndTabItem();
            }

            // Graph tab
            if (ImGui::BeginTabItem("Graph")) {

                ImGui::Text("Graph View of Results");
                ImGui::Separator();
                // Show message if no graph
                if (resultNodes.empty())
                {
                    ImGui::Text("No graph to display.");
                }
                else
                {
                    ImDrawList *draw_list = ImGui::GetWindowDrawList();
                    ImVec2 origin = ImGui::GetCursorScreenPos();
                    float x = origin.x + 50, y = origin.y + 50;
                    float y_step = 60;

                    // Draw nodes vertically as a tree-like view
                    for (size_t i = 0; i < resultNodes.size(); ++i)
                    {
                        ImVec2 node_pos(x, y + i * y_step);
                        // Draw node circle
                        draw_list->AddCircleFilled(node_pos, 10, IM_COL32(100, 200, 250, 255));
                        // Draw node label
                        draw_list->AddText(ImVec2(node_pos.x + 15, node_pos.y - 7), IM_COL32_WHITE, resultNodes[i].c_str());

                        // Draw edge to next node
                        if (i + 1 < resultNodes.size())
                        {
                            ImVec2 next_pos(x, y + (i + 1) * y_step);
                            draw_list->AddLine(node_pos, next_pos, IM_COL32(200, 200, 200, 255), 2.0f);
                        }
                    }
                    // Add vertical space after graph
                    ImGui::Dummy(ImVec2(0, resultNodes.size() * y_step + 20));
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
