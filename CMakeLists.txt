cmake_minimum_required(VERSION 3.10)
project(TebakAngka)

set(CMAKE_CXX_STANDARD 17)

if (MSVC)
    set(Boost_USE_STATIC_LIBS "On")
    find_package( Boost 1.52 COMPONENTS system thread regex REQUIRED )
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++1y -pedantic -Wextra")
    find_package( Boost 1.52 COMPONENTS system thread REQUIRED )
endif()

if(Boost_FOUND)
    message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
    message(STATUS "Boost_VERSION: ${Boost_VERSION}")

    include_directories(${Boost_INCLUDE_DIRS})
endif()

add_executable(TebakAngka main.cpp)

if(Boost_FOUND)
    target_link_libraries(TebakAngka ${Boost_LIBRARIES} -lpthread -lcurl)
endif()
