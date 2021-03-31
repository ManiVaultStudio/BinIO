cmake_minimum_required(VERSION 3.17)

set(PROJECT "BinIO")

PROJECT(${PROJECT})

add_definitions(-DQT_MESSAGELOGCONTEXT)

add_subdirectory(BinLoader)
#add_subdirectory(BinExporter)