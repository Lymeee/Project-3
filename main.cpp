// main.cpp
// Steam Review Comfort Visualizer

#include <iostream>
#include <vector>
#include <map>   // used for title to id match
#include <regex> // only for input check
#include <string>
#include <fstream>
#include <sstream>
#include <chrono> // used for timing the efficency of searches
#include <unordered_set>

#include "AdjacencyList.h"
// Struct for holding relevant Steam review data
struct Steam_Properties
{
    int appID;
    std::string name;
    std::string reviewText;
    int score; // review_score (1 = recommended, 0 = not)
    int votes; // review_votes (helpful votes)
};

// Data loading: parses CSV and returns vector of Steam_Properties
std::vector<Steam_Properties> loadReviews(const std::string &fileName, std::map<std::string, int> &uniqueTitles)
{
    std::ifstream file(fileName);
    std::vector<Steam_Properties> result;

    if (!file.is_open())
    {
        std::cerr << "Failed to open file.\n";
        return result;
    }

    std::string line;
    std::getline(file, line); // Skip header

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        Steam_Properties review;
        std::string field;

        try
        {
            // 1. app_id
            if (!std::getline(ss, field, ','))
                continue;
            review.appID = std::stoi(field);

            // 2. app_name
            if (!std::getline(ss, field, ','))
                continue;
            review.name = field;

            // Should repeat the comma code here, there are some games with commas in them. Other error I saw was invalid number, still trying to figure that one out

            // 3. review_text (quoted and may include commas)
            if (!std::getline(ss, field, ','))
                continue;
            if (!field.empty() && field.front() == '"')
            {
                std::string part;
                while (field.back() != '"' && std::getline(ss, part, ','))
                {
                    field += "," + part;
                }
                if (field.front() == '"' && field.back() == '"')
                {
                    field = field.substr(1, field.size() - 2);
                }
            }
            review.reviewText = field;

            // 4. review_score
            if (!std::getline(ss, field, ','))
                continue;
            review.score = std::stoi(field);

            // 5. review_votes
            if (!std::getline(ss, field, ','))
                continue;
            review.votes = std::stoi(field);

            result.push_back(review);

            // adds title to map with value as appID | added as a pass-by reference to save on re-iterating through whole vector lists again to find unique titles
            if (uniqueTitles.find(review.name) == uniqueTitles.end())
            {
                uniqueTitles.insert({review.name, review.appID});
            }
        }
        catch (const std::invalid_argument &e)
        {
            std::cerr << "Invalid number in row: " << line << "\n";
            continue;
        }
        catch (const std::out_of_range &e)
        {
            std::cerr << "Out of range number in row: " << line << "\n";
            continue;
        }
    }

    return result;
}

// Reverse Map: ID-Title
std::map<int, std::string> reverseTitleMap(const std::map<std::string, int> uniqueTitles)
{
    std::map<int, std::string> uniqueIDs;
    for (auto g : uniqueTitles)
    {
        uniqueIDs.insert({g.second, g.first});
    }
    return uniqueIDs;
}

// Filter reviews by review_score (recommended or not)
std::vector<Steam_Properties> filterByScore(const std::vector<Steam_Properties> &reviews, int desiredScore)
{
    std::vector<Steam_Properties> result;
    for (const auto &r : reviews)
    {
        if (r.score == desiredScore)
            result.push_back(r);
    }
    return result;
}

// Filter reviews by minimum number of helpful votes
std::vector<Steam_Properties> filterByVotes(const std::vector<Steam_Properties> &reviews, int minVotes)
{
    std::vector<Steam_Properties> result;
    for (const auto &r : reviews)
    {
        if (r.votes >= minVotes)
            result.push_back(r);
    }
    return result;
}

// Filters reviews by game title
std::vector<Steam_Properties> filterByGame(const std::vector<Steam_Properties> &reviews, int desiredGameID)
{
    std::vector<Steam_Properties> result;
    for (const auto &r : reviews)
    {
        if (r.appID == desiredGameID)
            result.push_back(r);
    }
    return result;
}

std::string cleanReviewText(std::string s)
{
    s.erase(0, s.find_first_not_of(" \t\r\n"));
    s.erase(s.find_last_not_of(" \t\r\n") + 1);
    return s;
}

void buildGraphFromReviews(const std::vector<Steam_Properties>& reviews,
                           AdjacencyList& graph) {
    // 1) bucket by appID
    std::unordered_map<int, std::vector<std::string>> byGame;
    for (auto& r : reviews) {
        byGame[r.appID].push_back(r.reviewText);
    }

    // 2) fully connect each bucket
    for (auto& [appID, texts] : byGame) {
        for (size_t i = 0; i < texts.size(); ++i) {
            for (size_t j = i + 1; j < texts.size(); ++j) {
                graph.makeEdge(texts[i], texts[j]);
            }
        }
    }
}


int main()
{
    std::vector<Steam_Properties> reviews;
    std::vector<Steam_Properties> filtered;
    std::map<std::string, int> uniqueTitles; // map organized with key as title for alphabetical reference
    std::map<int, std::string> uniqueIDs;    // id:title map for processing other choices

    bool loaded = false;
    std::string gameTitle = "N/A"; // filter check

    // Menu loop
    while (true)
    {
        std::cout << "\nSteam Review Comfort Visualizer\n"
                  << "1) Load data\n"
                  << "2) Apply filters\n"
                  << "3) Run BFS or DFS\n"
                  << "4) View stats\n"
                  << "5) Exit\n"
                  << "Enter choice: ";

        int choice;
        std::cin >> choice;

        // 1) Load CSV data
        if (choice == 1)
        {
            if (!loaded)
            {                                                            // wrapped this since I don't think we will need to keep reloading the same set
                reviews = loadReviews("steam_sample.csv", uniqueTitles); // Testing for now
                if (!reviews.empty())
                {
                    filtered = reviews;
                    uniqueIDs = reverseTitleMap(uniqueTitles);
                    loaded = true;
                    std::cout << "Loaded " << reviews.size() << " reviews.\n";
                }
                else
                {
                    std::cout << "Failed to load data.\n";
                }
            }
        }

        // 2) Apply filters to review data
        else if (choice == 2)
        {
            gameTitle = "N/A"; // resets game title/status
            if (!loaded)
            {
                std::cout << "Please load data first.\n";
            }
            else
            {
                int desiredScore, minVotes;
                std::string desiredGameID;

                // added to give a reference list for games and their id #
                std::cout << "App ID for Games\n"
                          << "======================\n";
                for (auto r : uniqueTitles)
                {
                    std::cout << r.first << ": " << r.second << std::endl;
                }
                std::cout << "======================\n";

                std::cout << "Game ID (ID number or 'q' to bypass): "; // currently, any char/string will work4
                std::cin >> desiredGameID;
                std::cout << "Filter by score (0 or 1): ";
                std::cin >> desiredScore;
                std::cout << "Minimum helpful votes: ";
                std::cin >> minVotes;

                // regex check for empty string input on filterByGame..., can also just use try/catch to avoid error
                if (std::regex_match(desiredGameID, std::regex("^[0-9]+$")))
                {
                    int gameID = std::stoi(desiredGameID);
                    gameTitle = uniqueIDs[gameID];
                    filtered = filterByGame(filtered, gameID);
                }

                filtered = filterByScore(filtered, desiredScore);
                filtered = filterByVotes(filtered, minVotes);

                std::cout << "Filtered to " << filtered.size() << " reviews.\n";
            }
        }

        // 3) Run BFS or DFS on graph (not fully implemented)
        else if (choice == 3)
        {
            if (!loaded)
            {
                std::cout << "Please load data first.\n";
                continue;
            }
            AdjacencyList new_graph;
            buildGraphFromReviews(reviews, new_graph); 

            std::cin.ignore();
            std::cout << "Enter search type (BFS or DFS): ";
            std::string search_type;
            std::getline(std::cin, search_type);

            std::cout << "Enter a keyword to search for:\n> ";
            std::string keyword;
            std::getline(std::cin, keyword);
            std::transform(keyword.begin(), keyword.end(), keyword.begin(), ::tolower);

            // find the first matching review in `reviews`
            std::string start_review;
            for (auto &r : reviews)
            {
                auto text = r.reviewText;
                std::string lower = text;
                std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower.find(keyword) != std::string::npos)
                {
                    start_review = text;
                    break;
                }
            }
            if (start_review.empty())
            {
                std::cout << "No review found containing \"" << keyword << "\".\n";
                continue;
            }

            std::cout << "Starting from review:\n\"" << start_review << "\"\n";
            auto t0 = std::chrono::high_resolution_clock::now();

            std::vector<std::string> output;
            if (search_type == "BFS" || search_type == "bfs")
                output = new_graph.BFS(start_review);
            else if (search_type == "DFS" || search_type == "dfs")
                output = new_graph.DFS(start_review);
            else
            {
                std::cout << "Invalid search type.\n";
                continue;
            }

            auto t1 = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);

            std::cout << "Search took " << duration.count() << " microseconds\n";
            for (auto &node : output)
            {
                std::cout << "- " << node << "\n";
            }
        }

        // 4) Display statistics of current filtered data
        else if (choice == 4)
        {
            if (!loaded || filtered.empty())
            {
                std::cout << "No reviews to show.\n";
            }
            else
            {
                int totalScore = 0, totalVotes = 0, recommended = 0;
                for (const auto &r : filtered)
                {
                    totalScore += r.score;
                    totalVotes += r.votes;
                    if (r.score == 1)
                        ++recommended;
                }

                double avgScore = static_cast<double>(totalScore) / filtered.size();
                double percentRecommended = 100.0 * recommended / filtered.size();

                if (gameTitle != "N/A")
                {
                    std::cout << "Viewing stats for " << gameTitle << std::endl;
                }
                else
                {
                    std::cout << "Total Distribution Stats:\n";
                }
                std::cout << "- Total Reviews: " << filtered.size() << "\n"
                          << "- Avg Score: " << avgScore << "\n"
                          << "- % Recommended: " << percentRecommended << "%\n"
                          << "- Helpful Votes: " << totalVotes << "\n";

                if (gameTitle != "N/A")
                {
                    for (const auto &r : filtered)
                    {
                        std::cout << r.reviewText << std::endl;
                    }
                }
            }
        }

        // 5) Exit
        else if (choice == 5)
        {
            std::cout << "Exiting program.\n";
            break;
        }

        // Invalid input
        else
        {
            std::cout << "Invalid option.\n";
        }
    }

    return 0;
}
