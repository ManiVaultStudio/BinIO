cmake_minimum_required(VERSION 3.17)

set(PROJECT "BinIO")

PROJECT(${PROJECT})

add_definitions(-DQT_MESSAGELOGCONTEXT)

# Get the env path and allow the user to change it
set(INSTALL_DIR $ENV{HDPS_INSTALL_DIR} CACHE PATH "The HDPS Install Directoy")
# Normalize the incoming install path
file(TO_CMAKE_PATH ${INSTALL_DIR} INSTALL_DIR)

add_subdirectory(BinLoader)
add_subdirectory(BinExporter)