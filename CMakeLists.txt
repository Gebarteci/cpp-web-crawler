cmake_minimum_required(VERSION 3.15)
project(WebCrawler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

include(FetchContent)

# --- cpr library for HTTP requests ---
FetchContent_Declare(
  cpr
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG 1.10.5
)
FetchContent_MakeAvailable(cpr)

add_executable(web_crawler src/main.cpp)

# We only need to link cpr now!
target_link_libraries(
  web_crawler
  PRIVATE
  cpr::cpr
)