cmake_minimum_required(VERSION 3.21)
project(AAA)

set(CMAKE_CXX_STANDARD 14)

add_executable(AAA main.cpp joker.cpp joker.h card.cpp card.h player.cpp player.h dealer.cpp dealer.h)
