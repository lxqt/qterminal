cmake_minimum_required( VERSION 2.6 )

project( qterminal )

option(USE_SYSTEM_QXT "Use system Qxt Library for global shortcuts" ON)


# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )


IF (NOT LIB_SUFFIX)
    MESSAGE(STATUS "")
    MESSAGE(STATUS "LIB_SUFFIX variable is not defined. It will be autodetected now.")
    MESSAGE(STATUS "You can set it manually with -DLIB_SUFFIX=<value> (64 for example)")
    IF (NOT APPLE)
        # check 64 bit
        if (CMAKE_SIZEOF_VOID_P EQUAL 4)
            SET (LIB_SUFFIX "")
        else (CMAKE_SIZEOF_VOID_P EQUAL 4)
            set (LIB_SUFFIX "64")
            MESSAGE(STATUS "   LIB_SUFFIX is set to '${LIB_SUFFIX}'")
        endif (CMAKE_SIZEOF_VOID_P EQUAL 4)
    ELSE (NOT APPLE)
        message(STATUS "LIB_SUFFIX is disabled for APPLE: ${CMAKE_SYSTEM_NAME}")
        SET (LIB_SUFFIX "")
    ENDIF (NOT APPLE)
    # BSD does not use lib64 for 64bit libs
    IF (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD" OR ${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD")
        message(STATUS "LIB_SUFFIX is disabled for *BSD: ${CMAKE_SYSTEM_NAME}")
        SET (LIB_SUFFIX "")
    ENDIF()
    #
    MESSAGE(STATUS "")
ENDIF (NOT LIB_SUFFIX)


find_package( Qt4 COMPONENTS QtCore QtGui REQUIRED)
MESSAGE(STATUS "Qt4 version: ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}")
add_definitions ( ${QT_DEFINITIONS} )
include_directories( ${QT_INCLUDES} )
INCLUDE( ${QT_USE_FILE} )
MESSAGE(STATUS "Qt4 libs: ${QT_LIBRARIES}")


if (USE_SYSTEM_QXT)
    message(STATUS "Using system Qxt...")
    find_package(Qxt REQUIRED COMPONENTS QxtCore QxtGui)
    if (NOT QXT_FOUND)
        message(FATAL_ERROR "System Qxt not found. Install Qxt devel package or use -DUSE_SYSTEM_QXT=0 to build with internal copy of this library")
    endif (NOT QXT_FOUND)

    set(QXT_INCLUDE_DIRS ${QXT_INCLUDE_PATH}/QxtCore ${QXT_INCLUDE_PATH}/QxtGui)
else (USE_SYSTEM_QXT)
    message(STATUS "Using bundled Qxt...")
    set(QXT_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src/third-party)
endif (USE_SYSTEM_QXT)


find_package ( QTermWidget REQUIRED )


if (APPLE)
   FIND_LIBRARY(CARBON_LIBRARY Carbon REQUIRED)
   MESSAGE(STATUS "CARBON_LIBRARY: ${CARBON_LIBRARY}")
elseif (UNIX)
   FIND_PACKAGE(X11 REQUIRED)
   MESSAGE(STATUS "X11_X11_LIB: ${X11_X11_LIB}")
endif ()

set(STR_VERSION "0.4.0")
add_definitions(-DSTR_VERSION=\"${STR_VERSION}\")

# a helper for macosx to build 32/64 bit universals
IF (WANT_UNIVERSAL)
    SET (CMAKE_OSX_ARCHITECTURES "i386;x86_64;")
    MESSAGE(STATUS "Archs builds: ${CMAKE_OSX_ARCHITECTURES}")
ENDIF (WANT_UNIVERSAL)

subdirs( src )


install(FILES qterminal.desktop DESTINATION share/applications)
install(FILES qterminal_drop.desktop DESTINATION share/applications)


# make dist custom target
SET(CPACK_PACKAGE_NAME "qterminal")
SET(CPACK_PACKAGE_VERSION ${STR_VERSION})
SET(CPACK_SOURCE_GENERATOR "TGZ;TBZ2")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
SET(CPACK_IGNORE_FILES "/CVS/;/\\\\.svn/;/\\\\.git/;\\\\.swp$;\\\\.#;/#;\\\\.tar.gz$;/CMakeFiles/;CMakeCache.txt;\\\\.qm$;/build/;\\\\.diff$;.DS_Store'")
SET(CPACK_SOURCE_IGNORE_FILES ${CPACK_IGNORE_FILES})
INCLUDE(CPack)
# simulate autotools' "make dist"
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

