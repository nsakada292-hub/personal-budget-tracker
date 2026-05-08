cmake_minimum_required(VERSION 3.14)
project(PersonalBudgetManager)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# =============================================
#  FetchContent - downloads all libraries
#  automatically when you build!
# =============================================
include(FetchContent)

# 1. nlohmann/json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# 2. tabulate (pretty tables)
FetchContent_Declare(
    tabulate
    GIT_REPOSITORY https://github.com/p-ranav/tabulate.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(tabulate)

# 3. cxxopts (command line options)
FetchContent_Declare(
    cxxopts
    GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
    GIT_TAG        v3.1.1
)
FetchContent_MakeAvailable(cxxopts)

# =============================================
#  Build the executable
# =============================================
add_executable(budget
    src/main.cpp
    src/Transaction.cpp
    src/AuthManager.cpp
    src/BudgetManager.cpp
)

# Include headers
target_include_directories(budget PRIVATE include)

# Link all libraries
target_link_libraries(budget
    PRIVATE
    nlohmann_json::nlohmann_json
    tabulate::tabulate
    cxxopts::cxxopts
)