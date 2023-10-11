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
# Check if the directory to the ManiVault installation has been provided
if(NOT DEFINED MV_INSTALL_DIR)
    set(MV_INSTALL_DIR "" CACHE PATH "Directory where ManiVault is installed")
    message(FATAL_ERROR "Please set MV_INSTALL_DIR to the directory where ManiVault is installed")
endif()
file(TO_CMAKE_PATH ${MV_INSTALL_DIR} MV_INSTALL_DIR)

# -----------------------------------------------------------------------------
# Plugins
# -----------------------------------------------------------------------------
add_subdirectory(BinLoader)
add_subdirectory(BinExporter)
