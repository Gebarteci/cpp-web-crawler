cmake_minimum_required(VERSION 3.15)
project(WebCrawler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread") # Add pthread flag for threads

include(FetchContent)

# --- cpr library for HTTP requests ---
FetchContent_Declare(
  cpr
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG 1.10.5
)

# --- Gumbo library for HTML parsing ---
FetchContent_Declare(
  gumbo-parser
  GIT_REPOSITORY https://github.com/google/gumbo-parser.git
  GIT_TAG v0.10.1
)

FetchContent_MakeAvailable(cpr gumbo-parser)

add_executable(web_crawler src/main.cpp)

# Link all necessary libraries
target_link_libraries(
  web_crawler
  PRIVATE
  cpr::cpr
  gumbo-parser
)