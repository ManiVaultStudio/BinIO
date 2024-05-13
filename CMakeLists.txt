cmake_minimum_required(VERSION 3.17)

# -----------------------------------------------------------------------------
# BinIO Plugins
# -----------------------------------------------------------------------------
set(PROJECT "BinIO")
PROJECT(${PROJECT})

add_definitions(-DQT_MESSAGELOGCONTEXT)

message(STATUS "BINIO: ${ManiVault_DIR}")

# -----------------------------------------------------------------------------
# Plugins
# -----------------------------------------------------------------------------
add_subdirectory(BinLoader)
add_subdirectory(BinExporter)
