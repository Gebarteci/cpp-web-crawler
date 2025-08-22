cmake_minimum_required(VERSION 3.15)
project(WebCrawler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-independent threading
find_package(Threads REQUIRED)

include(FetchContent)

# --- cpr library for HTTP requests ---
FetchContent_Declare(
  cpr
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG 1.10.5
)
FetchContent_MakeAvailable(cpr)

add_executable(web_crawler src/main.cpp)

target_link_libraries(
  web_crawler
  PRIVATE
  cpr::cpr
  Threads::Threads  # Platform-independent threading
)