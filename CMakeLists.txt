cmake_minimum_required(VERSION 3.5.0)
project(Demon VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

add_executable(Demon main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

target_link_libraries(${PROJECT_NAME} enet)