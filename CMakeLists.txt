cmake_minimum_required(VERSION 3.17)

# -----------------------------------------------------------------------------
# BinIO Plugins
# -----------------------------------------------------------------------------
set(PROJECT "BinIO")
PROJECT(${PROJECT})

add_definitions(-DQT_MESSAGELOGCONTEXT)

# -----------------------------------------------------------------------------
# Set install directory
# -----------------------------------------------------------------------------
# Get the env path and allow the user to change it
set(INSTALL_DIR $ENV{HDPS_INSTALL_DIR} CACHE PATH "The HDPS Install Directoy")
# Normalize the incoming install path
file(TO_CMAKE_PATH ${INSTALL_DIR} INSTALL_DIR)

# -----------------------------------------------------------------------------
# Plugins
# -----------------------------------------------------------------------------
add_subdirectory(BinLoader)
add_subdirectory(BinExporter)
