// main.cpp
// Steam Review Comfort Visualizer

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>


// Struct for holding relevant Steam review data
struct Steam_Properties {
    int appID;
    std::string name;
    std::string reviewText;
    int score;      // review_score (1 = recommended, 0 = not)
    int votes;      // review_votes (helpful votes)
};

// Data loading: parses CSV and returns vector of Steam_Properties
std::vector<Steam_Properties> loadReviews(const std::string& fileName) {
    std::ifstream file(fileName);
    std::vector<Steam_Properties> result;

    if (!file.is_open()) {
        std::cerr << "Failed to open file.\n";
        return result;
    }

    std::string line;
    std::getline(file, line); // Skip header

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        Steam_Properties review;
        std::string field;

        try {
            // 1. app_id
            if (!std::getline(ss, field, ',')) continue;
            review.appID = std::stoi(field);

            // 2. app_name
            if (!std::getline(ss, field, ',')) continue;
            review.name = field;

            // 3. review_text (quoted and may include commas)
            if (!std::getline(ss, field, ',')) continue;
            if (field.front() == '"') {
                std::string part;
                while (field.back() != '"' && std::getline(ss, part, ',')) {
                    field += "," + part;
                }
                if (field.front() == '"' && field.back() == '"') {
                    field = field.substr(1, field.size() - 2);
                }
            }
            review.reviewText = field;

            // 4. review_score
            if (!std::getline(ss, field, ',')) continue;
            review.score = std::stoi(field);

            // 5. review_votes
            if (!std::getline(ss, field, ',')) continue;
            review.votes = std::stoi(field);

            result.push_back(review);
        }
        catch (const std::invalid_argument& e) {
            std::cerr << "Invalid number in row: " << line << "\n";
            continue;
        }
        catch (const std::out_of_range& e) {
            std::cerr << "Out of range number in row: " << line << "\n";
            continue;
        }
    }

    return result;
}


// Filter reviews by review_score (recommended or not)
std::vector<Steam_Properties> filterByScore(const std::vector<Steam_Properties>& reviews, int desiredScore) {
    std::vector<Steam_Properties> result;
    for (const auto& r : reviews) {
        if (r.score == desiredScore) result.push_back(r);
    }
    return result;
}

// Filter reviews by minimum number of helpful votes
std::vector<Steam_Properties> filterByVotes(const std::vector<Steam_Properties>& reviews, int minVotes) {
    std::vector<Steam_Properties> result;
    for (const auto& r : reviews) {
        if (r.votes >= minVotes) result.push_back(r);
    }
    return result;
}

int main() {
    std::vector<Steam_Properties> reviews;
    std::vector<Steam_Properties> filtered;
    bool loaded = false;

    // Menu loop
    while (true) {
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
        if (choice == 1) {
            reviews = loadReviews("../steam_sample.csv"); // Testing for now
            if (!reviews.empty()) {
                filtered = reviews;
                loaded = true;
                std::cout << "Loaded " << reviews.size() << " reviews.\n";
            } else {
                std::cout << "Failed to load data.\n";
            }
        }

        // 2) Apply filters to review data
        else if (choice == 2) {
            if (!loaded) {
                std::cout << "Please load data first.\n";
            } else {
                int desiredScore, minVotes;
                std::cout << "Filter by score (0 or 1): ";
                std::cin >> desiredScore;
                std::cout << "Minimum helpful votes: ";
                std::cin >> minVotes;

                filtered = filterByScore(reviews, desiredScore);
                filtered = filterByVotes(filtered, minVotes);
                std::cout << "Filtered to " << filtered.size() << " reviews.\n";
            }
        }

        // 3) Run BFS or DFS on graph (not implemented)
        else if (choice == 3) {
            if (!loaded) std::cout << "Please load data first.\n";
            else std::cout << "Search (BFS/DFS) not implemented yet.\n";
        }

        // 4) Display statistics of current filtered data
        else if (choice == 4) {
            if (!loaded || filtered.empty()) {
                std::cout << "No reviews to show.\n";
            } else {
                int totalScore = 0, totalVotes = 0, recommended = 0;
                for (const auto& r : filtered) {
                    totalScore += r.score;
                    totalVotes += r.votes;
                    if (r.score == 1) ++recommended;
                }

                double avgScore = static_cast<double>(totalScore) / filtered.size();
                double percentRecommended = 100.0 * recommended / filtered.size();

                std::cout << "Stats:\n"
                          << "- Total Reviews: " << filtered.size() << "\n"
                          << "- Avg Score: " << avgScore << "\n"
                          << "- % Recommended: " << percentRecommended << "%\n"
                          << "- Helpful Votes: " << totalVotes << "\n";
            }
        }

        // 5) Exit
        else if (choice == 5) {
            std::cout << "Exiting program.\n";
            break;
        }

        // Invalid input
        else {
            std::cout << "Invalid option.\n";
        }
    }

    return 0;
}
