cmake_minimum_required( VERSION 2.6 )

project( qterminal )

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

find_package( Qt4 REQUIRED )
INCLUDE( ${QT_USE_FILE} )
MESSAGE(STATUS "Qt4 version: ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}")
add_definitions ( ${QT_DEFINITIONS} )
include_directories( ${QT_INCLUDES} )

find_package ( QTermWidget REQUIRED )

subdirs( src )

