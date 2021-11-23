cmake_minimum_required(VERSION 3.17)
project(final-proj VERSION 0.1.0)

# include(CTest)
# enable_testing()

add_subdirectory(${PROJECT_SOURCE_DIR}/src/THC-chess EXCLUDE_FROM_ALL)
include_directories(${THC_CHESS_INCLUDE} ./src/UCI ./src/MCTS ./src)


add_executable(final-proj src/main.cpp 
    src/utils.cpp 
    src/UCI/UCI.cpp 
    src/MCTS/MCTS_root.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(final-proj PRIVATE Threads::Threads thc_chess_static)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O3 -mavx2")
include(CPack)
