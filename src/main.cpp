#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <set>
#include <utility>
#include <regex> // Required for regular expressions

#include <cpr/cpr.h>

// --- HTTP Function (remains the same) ---
std::string download_page(const std::string& url) {
    cpr::Response r = cpr::Get(cpr::Url{url});
    if (r.status_code == 200) {
        return r.text;
    }
    return "";
}

// --- NEW: URL Resolution Function ---
// Converts relative URLs (e.g., "/about") to absolute URLs.
std::string resolve_url(const std::string& base_url, const std::string& link) {
    if (link.rfind("http", 0) == 0) {
        return link; // Already an absolute link
    }

    // Find the end of the domain name
    size_t end_of_domain = base_url.find('/', 8); // Start search after "https://"
    std::string domain = base_url.substr(0, end_of_domain);

    if (link.rfind("/", 0) == 0) {
        return domain + link; // Link starts with '/', relative to domain root
    }
    
    // For this project, we'll ignore more complex relative links
    return ""; 
}

// --- Parsing Function (Updated to use Regex) ---
std::set<std::string> find_links(const std::string& html_body) {
    std::set<std::string> links;
    // This regex finds href attributes in <a> tags
    std::regex link_regex("<a\\s+[^>]*href\\s*=\\s*[\"'](.*?)[\"']");
    
    // Use an iterator to find all matches in the HTML
    auto words_begin = std::sregex_iterator(html_body.begin(), html_body.end(), link_regex);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        if (match.size() > 1) {
            links.insert(match[1].str()); // match[1] is the captured group (the URL)
        }
    }
    return links;
}

// --- Main Loop (Updated to use resolve_url) ---
int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: ./web_crawler <starting_url> <depth>" << std::endl;
        return 1;
    }

    std::string start_url = argv[1];
    int max_depth = std::stoi(argv[2]);

    std::queue<std::pair<std::string, int>> to_visit;
    to_visit.push({start_url, 0});

    std::set<std::string> visited;

    while (!to_visit.empty()) {
        auto [current_url, current_depth] = to_visit.front();
        to_visit.pop();

        if (visited.count(current_url)) {
            continue;
        }
        
        if (current_depth > max_depth) {
            continue;
        }

        std::cout << "[Depth " << current_depth << "] Crawling: " << current_url << std::endl;
        visited.insert(current_url);

        std::string html = download_page(current_url);
        if (html.empty()) {
            std::cout << "  -> Failed to download." << std::endl;
            continue;
        }

        std::set<std::string> new_links = find_links(html);
        std::cout << "  -> Found " << new_links.size() << " links." << std::endl;

        for (const auto& link : new_links) {
            // Resolve the link to an absolute URL
            std::string absolute_link = resolve_url(current_url, link);
            if (!absolute_link.empty() && visited.find(absolute_link) == visited.end()) {
                to_visit.push({absolute_link, current_depth + 1});
            }
        }
    }

    std::cout << "\nCrawling finished. Visited " << visited.size() << " unique pages." << std::endl;
    return 0;
}
