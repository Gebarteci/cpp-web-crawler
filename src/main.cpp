#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <set>
#include <map>
#include <utility>
#include <regex>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <fstream>

#include <cpr/cpr.h>

// --- Shared data structures ---
struct CrawlerState {
    std::mutex mtx; 
    std::queue<std::pair<std::string, int>> to_visit;
    std::set<std::string> visited;
    std::map<int, std::vector<std::pair<std::string, bool>>> results_by_depth;
};

std::string download_page(const std::string& url) {
    try {
        cpr::Response r = cpr::Get(cpr::Url{url});
        if (r.status_code == 200) {
            return r.text;
        } else {
            std::cerr << "[Error] Failed to download " << url 
                      << " (Status code: " << r.status_code << ")" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "[Error] Exception while downloading " << url 
                  << ": " << e.what() << std::endl;
    }
    return "";
}

std::string resolve_url(const std::string& base_url, const std::string& link) {
    if (link.rfind("http", 0) == 0) return link;
    size_t end_of_domain = base_url.find('/', 8);
    std::string domain = base_url.substr(0, end_of_domain);
    if (link.rfind("/", 0) == 0) return domain + link;
    return ""; 
}

std::set<std::string> find_links(const std::string& html_body) {
    std::set<std::string> links;
    std::regex link_regex("<a\\s+[^>]*href\\s*=\\s*[\"'](.*?)[\"']");
    auto words_begin = std::sregex_iterator(html_body.begin(), html_body.end(), link_regex);
    auto words_end = std::sregex_iterator();
    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        if (i->size() > 1) links.insert((*i)[1].str());
    }
    return links;
}

void worker(int id, CrawlerState& state, int max_depth, std::atomic<int>& tasks_in_progress) {
    while (true) {
        std::pair<std::string, int> current_task;
        bool have_task = false;

        {
            std::lock_guard<std::mutex> guard(state.mtx);
            if (state.to_visit.empty()) {
                if (tasks_in_progress.load() == 0) {
                    return;
                }
            } else {
                current_task = state.to_visit.front();
                state.to_visit.pop();

                if (state.visited.count(current_task.first)) {
                    continue;
                }

                state.visited.insert(current_task.first);
                tasks_in_progress++;
                have_task = true;
            }
        }

        if (have_task) {
            auto [current_url, current_depth] = current_task;

            if (current_depth <= max_depth) {
                std::cout << "[Thread " << id << "][Depth " << current_depth << "] Crawling: " << current_url << std::endl;
                
                std::string html = download_page(current_url);
                bool success = !html.empty();

                {
                    std::lock_guard<std::mutex> guard(state.mtx);
                    state.results_by_depth[current_depth].push_back({current_url, success});
                }

                if (success) {
                    std::set<std::string> new_links = find_links(html);
                    {
                        std::lock_guard<std::mutex> guard(state.mtx);
                        for (const auto& link : new_links) {
                            std::string absolute_link = resolve_url(current_url, link);
                            if (!absolute_link.empty()) {
                                state.to_visit.push({absolute_link, current_depth + 1});
                            }
                        }
                    }
                } else {
                    std::cout << "[Thread " << id << "][Depth " << current_depth 
                             << "] Failed to process: " << current_url << std::endl;
                }
            }
            tasks_in_progress--;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: ./web_crawler <starting_url> <depth>" << std::endl;
        return 1;
    }

    std::string start_url = argv[1];
    int max_depth = std::stoi(argv[2]);

    const unsigned int num_threads = std::thread::hardware_concurrency();
    std::cout << "Using " << num_threads << " threads for crawling." << std::endl;

    CrawlerState state;
    state.to_visit.push({start_url, 0});

    std::vector<std::thread> threads;
    std::atomic<int> tasks_in_progress = {0};

    for (unsigned int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker, i + 1, std::ref(state), max_depth, std::ref(tasks_in_progress));
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "\nCrawling finished. Visited " << state.visited.size() << " unique pages." << std::endl;

    // Save processed URLs
    std::cout << "Saving processed results to results.txt..." << std::endl;
    std::ofstream output_file("results.txt");
    if (output_file.is_open()) {
        output_file << "Total visited URLs: " << state.visited.size() << "\n\n";
        
        for (const auto& pair : state.results_by_depth) {
            output_file << "--- Depth " << pair.first << " ---\n";
            
            int success_count = 0;
            int failed_count = 0;
            
            for (const auto& [url, success] : pair.second) {
                output_file << (success ? "[Success] " : "[Failed]  ") << url << "\n";
                success ? success_count++ : failed_count++;
            }
            
            output_file << "\nDepth " << pair.first << " Summary:\n";
            output_file << "Successful: " << success_count << "\n";
            output_file << "Failed: " << failed_count << "\n";
            output_file << "Total: " << success_count + failed_count << "\n\n";
        }
        output_file.close();
        std::cout << "Successfully saved processed results." << std::endl;
    } else {
        std::cerr << "Error: Could not open results.txt for writing." << std::endl;
    }

    // Save all visited URLs
    std::cout << "Saving all visited URLs to all_visited.txt..." << std::endl;
    std::ofstream visited_file("all_visited.txt");
    if (visited_file.is_open()) {
        visited_file << "Total visited URLs: " << state.visited.size() << "\n\n";
        
        // Write all URLs from the visited set
        for (const auto& url : state.visited) {
            // Check if this URL was processed (exists in results_by_depth)
            bool was_processed = false;
            for (const auto& [depth, urls] : state.results_by_depth) {
                for (const auto& [processed_url, success] : urls) {
                    if (processed_url == url) {
                        was_processed = true;
                        break;
                    }
                }
                if (was_processed) break;
            }
            visited_file << (was_processed ? "[Processed] " : "[Unprocessed] ") << url << "\n";
        }
        visited_file.close();
        std::cout << "Successfully saved all visited URLs." << std::endl;
    } else {
        std::cerr << "Error: Could not open all_visited.txt for writing." << std::endl;
    }

    return 0;
}