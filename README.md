# C++ Web Crawler

A multi-threaded web crawler implemented in C++ that efficiently crawls websites and saves results organized by depth level.

## Features

- Multi-threaded crawling using C++17 standard
- Depth-limited crawling
- URL deduplication
- Support for relative and absolute URLs
- Progress tracking and statistics
- Output files with detailed crawling results
- Uses CPR library for HTTP requests

## Requirements

- CMake (version 3.15 or higher)
- C++17 compatible compiler
- pthread/Win32 threads support (project uses cpp standard thread)
- Internet connection
- CPR library (automatically fetched by CMake)

## Build Instructions

1. Clone the repository:
```bash
git clone https://github.com/yourusername/cpp-web-crawler.git
cd cpp-web-crawler
```

2. Create and enter the build directory:
```bash
mkdir build && cd build
```

3. Build the project:
```bash
cmake ..
make
```

## Usage

Run the crawler with a starting URL and maximum depth:

```bash
./web_crawler <starting_url> <depth>
```

Example:
```bash
./web_crawler http://quotes.toscrape.com 2
```

## Output Files

The crawler generates two output files:

1. `results.txt`:
   - Contains successfully processed URLs organized by depth
   - Shows success/failure status for each URL
   - Includes summary statistics for each depth level

2. `all_visited.txt`:
   - Lists all URLs encountered during crawling
   - Marks URLs as either [Processed] or [Unprocessed]
   - Shows total number of unique URLs visited

## Output Format

### results.txt
```
Total visited URLs: <count>

--- Depth 0 ---
[Success] http://example.com
...

Depth 0 Summary:
Successful: X
Failed: Y
Total: Z
```

### all_visited.txt
```
Total visited URLs: <count>

[Processed] http://example.com
[Unprocessed] http://example.com/page
...
```

## Performance

- Uses system's maximum available threads
- Implements mutex-based thread synchronization
- Efficient URL storage and duplicate checking
- Asynchronous HTTP requests

## Limitations

- Respects robots.txt implicitly through CPR
- No delay between requests to the same domain
- Maximum depth should be set reasonably to avoid excessive crawling

## License

X

## Contributing

Feel free to submit issues, fork the repository, and create pull requests for any improvements.
