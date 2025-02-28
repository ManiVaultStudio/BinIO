cmake_minimum_required(VERSION 3.22)

# -----------------------------------------------------------------------------
# BinIO Plugins
# -----------------------------------------------------------------------------
set(PROJECT "BinIO")
PROJECT(${PROJECT})

add_definitions(-DQT_MESSAGELOGCONTEXT)

# -----------------------------------------------------------------------------
# Plugins
# -----------------------------------------------------------------------------
add_subdirectory(BinLoader)
add_subdirectory(BinExporter)
