cmake_minimum_required(VERSION 3.23)
project(retro_game C)

set(CMAKE_C_STANDARD 99)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

add_executable(retro_game main.c main.h)
