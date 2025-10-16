cmake_minimum_required(VERSION 3.22)

# -----------------------------------------------------------------------------
# BinExporter Plugin
# -----------------------------------------------------------------------------
set(BINEXPORTER "BinExporter")
PROJECT(${BINEXPORTER} 
        LANGUAGES CXX)

# -----------------------------------------------------------------------------
# CMake Options
# -----------------------------------------------------------------------------
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /DWIN32 /EHsc /MP /permissive- /Zc:__cplusplus")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MD")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /DEBUG")
endif(MSVC)

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------
find_package(Qt6 COMPONENTS Widgets WebEngineWidgets REQUIRED)

find_package(ManiVault COMPONENTS Core PointData CONFIG QUIET)

# -----------------------------------------------------------------------------
# Source files
# -----------------------------------------------------------------------------
set(SOURCES
    src/BinExporter.h
    src/BinExporter.cpp
)

set(AUX
    PluginInfo.json
)

source_group(Plugin FILES ${SOURCES})
source_group(Aux FILES ${AUX})

# -----------------------------------------------------------------------------
# CMake Target
# -----------------------------------------------------------------------------
add_library(${BINEXPORTER} SHARED ${SOURCES} ${AUX})

# -----------------------------------------------------------------------------
# Target include directories
# -----------------------------------------------------------------------------
target_include_directories(${BINEXPORTER} PRIVATE "${ManiVault_INCLUDE_DIR}")

# -----------------------------------------------------------------------------
# Target properties
# -----------------------------------------------------------------------------
target_compile_features(${BINEXPORTER} PRIVATE cxx_std_20)

# -----------------------------------------------------------------------------
# Target library linking
# -----------------------------------------------------------------------------
target_link_libraries(${BINEXPORTER} PRIVATE Qt6::Widgets)
target_link_libraries(${BINEXPORTER} PRIVATE Qt6::WebEngineWidgets)
target_link_libraries(${BINEXPORTER} PRIVATE ManiVault::Core)
target_link_libraries(${BINEXPORTER} PRIVATE ManiVault::PointData)

# -----------------------------------------------------------------------------
# Target installation
# -----------------------------------------------------------------------------
install(TARGETS ${BINEXPORTER}
    RUNTIME DESTINATION Plugins COMPONENT PLUGINS # Windows .dll
    LIBRARY DESTINATION Plugins COMPONENT PLUGINS # Linux/Mac .so
)

add_custom_command(TARGET ${BINEXPORTER} POST_BUILD
    COMMAND "${CMAKE_COMMAND}"
    --install ${CMAKE_CURRENT_BINARY_DIR}
    --config $<CONFIGURATION>
    --component PLUGINS
    --prefix ${ManiVault_INSTALL_DIR}/$<CONFIGURATION>
)

mv_handle_plugin_config(${BINEXPORTER})

# -----------------------------------------------------------------------------
# Miscellaneous
# -----------------------------------------------------------------------------
# Automatically set the debug environment (command + working directory) for MSVC
if(MSVC)
    set_property(TARGET ${BINEXPORTER} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<IF:$<CONFIG:DEBUG>,${ManiVault_INSTALL_DIR}/Debug,$<IF:$<CONFIG:RELWITHDEBINFO>,${ManiVault_INSTALL_DIR}/RelWithDebInfo,${ManiVault_INSTALL_DIR}/Release>>)
    set_property(TARGET ${BINEXPORTER} PROPERTY VS_DEBUGGER_COMMAND $<IF:$<CONFIG:DEBUG>,"${ManiVault_INSTALL_DIR}/Debug/ManiVault Studio.exe",$<IF:$<CONFIG:RELWITHDEBINFO>,"${ManiVault_INSTALL_DIR}/RelWithDebInfo/ManiVault Studio.exe","${ManiVault_INSTALL_DIR}/Release/ManiVault Studio.exe">>)
endif()




function(add_post_build_copy_pdbs_rwd tgt dest_dir)
  if(NOT MSVC)
    return()
  endif()

  # Create destination dir (only in RwDI)
  add_custom_command(TARGET ${tgt} POST_BUILD
    COMMAND
      $<$<CONFIG:RelWithDebInfo>:${CMAKE_COMMAND}>
      $<$<CONFIG:RelWithDebInfo>:-E>
      $<$<CONFIG:RelWithDebInfo>:make_directory>
      $<$<CONFIG:RelWithDebInfo>:"${dest_dir}">
    COMMAND_EXPAND_LISTS
    VERBATIM)

  # Copy linker PDB (EXE/DLL) if it exists, only in RwDI
  add_custom_command(TARGET ${tgt} POST_BUILD
    COMMAND
      $<$<AND:$<CONFIG:RelWithDebInfo>,$<BOOL:$<TARGET_PDB_FILE:${tgt}>>>:${CMAKE_COMMAND}>
      $<$<AND:$<CONFIG:RelWithDebInfo>,$<BOOL:$<TARGET_PDB_FILE:${tgt}>>>:-E>
      $<$<AND:$<CONFIG:RelWithDebInfo>,$<BOOL:$<TARGET_PDB_FILE:${tgt}>>>:copy_if_different>
      $<$<AND:$<CONFIG:RelWithDebInfo>,$<BOOL:$<TARGET_PDB_FILE:${tgt}>>>:"$<TARGET_PDB_FILE:${tgt}>">
      $<$<AND:$<CONFIG:RelWithDebInfo>,$<BOOL:$<TARGET_PDB_FILE:${tgt}>>>:"${dest_dir}/$<TARGET_FILE_BASE_NAME:${tgt}>.pdb">
    COMMAND_EXPAND_LISTS
    VERBATIM)

  # Copy compile PDB (useful for static libs), only in RwDI
  add_custom_command(TARGET ${tgt} POST_BUILD
    COMMAND
      $<$<AND:$<CONFIG:RelWithDebInfo>,$<BOOL:$<TARGET_COMPILE_PDB_FILE:${tgt}>>>:${CMAKE_COMMAND}>
      $<$<AND:$<CONFIG:RelWithDebInfo>,$<BOOL:$<TARGET_COMPILE_PDB_FILE:${tgt}>>>:-E>
      $<$<AND:$<CONFIG:RelWithDebInfo>,$<BOOL:$<TARGET_COMPILE_PDB_FILE:${tgt}>>>:copy_if_different>
      $<$<AND:$<CONFIG:RelWithDebInfo>,$<BOOL:$<TARGET_COMPILE_PDB_FILE:${tgt}>>>:"$<TARGET_COMPILE_PDB_FILE:${tgt}>">
      $<$<AND:$<CONFIG:RelWithDebInfo>,$<BOOL:$<TARGET_COMPILE_PDB_FILE:${tgt}>>>:"${dest_dir}/$<TARGET_NAME:${tgt}>.compile.pdb">
    COMMAND_EXPAND_LISTS
    VERBATIM)
endfunction()

add_post_build_copy_pdbs_rwd(${BINEXPORTER} "D:/pdbs/")
