cmake_minimum_required( VERSION 2.6 )

project( qterminal )

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

find_package( Qt4 COMPONENTS QtCore QtGui REQUIRED)
INCLUDE( ${QT_USE_FILE} )
MESSAGE(STATUS "Qt4 version: ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}")
add_definitions ( ${QT_DEFINITIONS} )
include_directories( ${QT_INCLUDES} )
MESSAGE(STATUS "Qt4 libs: ${QT_LIBRARIES}")

find_package ( QTermWidget REQUIRED )

set(STR_VERSION "0.2.0")
add_definitions(-DSTR_VERSION=\"${STR_VERSION}\")

# a helper for macosx to build 32/64 bit universals
IF (WANT_UNIVERSAL)
    SET (CMAKE_OSX_ARCHITECTURES "i386;x86_64;")
    MESSAGE(STATUS "Archs builds: ${CMAKE_OSX_ARCHITECTURES}")
ENDIF (WANT_UNIVERSAL)

subdirs( src )
