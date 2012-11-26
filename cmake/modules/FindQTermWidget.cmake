# - Find QTermWidget
# Find the QTermWidget includes and client library
# This module defines
#  QTERMWIDGET_INCLUDE_DIR, where to find includes
#  QTERMWIDGET_LIBRARIES, the libraries needed to use qtermwidget
#  QTERMWIDGET_SHARE, the directory containing required share files. Propably usable only for mac bundles.
#  QTERMWIDGET_FOUND, the flag id the system contains qtermwidget library
#
# Petr Vanek <petr@scribus.info>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if (QTERMWIDGET_INCLUDE_DIR AND QTERMWIDGET_LIBRARIES)
  # Already in cache, be silent
  set(PostgreSQL_FIND_QUIETLY TRUE)
endif (QTERMWIDGET_INCLUDE_DIR AND QTERMWIDGET_LIBRARIES)


message(STATUS "QTermWidget: searching for includes...")
find_path(QTERMWIDGET_INCLUDE_DIR qtermwidget.h
       ${QTERMWIDGET_PATH_INCLUDES}/
       /usr/include/
       /usr/local/include/
       /opt/local/include/
)
message(STATUS "             DONE")

message(STATUS "QTermWidget: searching for libs...")
find_library(QTERMWIDGET_LIBRARIES NAMES qtermwidget libqtermwidget
    PATHS
        ${QTERMWIDGET_PATH_LIB}
        /usr/lib/
    	/usr/lib${LIB_SUFFIX}/
	    /usr/local/lib/
        /usr/local/lib${LIB_SUFFIX}/
        /opt/local/lib/
)
message(STATUS "             DONE")

message(STATUS "QTermWidget: searching for share dir location...")
find_path(QTERMWIDGET_SHARE qtermwidget
    PATHS
        ${QTERMWIDGET_PATH_SHARE}
        ${QTERMWIDGET_INCLUDE_DIR}/../share/
        /usr/share/
        /usr/local/share/
        /opt/local/share/
)
message(STATUS "             DONE")


mark_as_advanced(QTERMWIDGET_INCLUDE_DIR QTERMWIDGET_LIBRARIES QTERMWIDGET_SHARE)

if (QTERMWIDGET_LIBRARIES AND QTERMWIDGET_INCLUDE_DIR AND QTERMWIDGET_SHARE)
    set( QTERMWIDGET_FOUND 1 )
endif()


IF (QTERMWIDGET_FOUND)

    MESSAGE(STATUS "QTermWidget found")
    MESSAGE(STATUS " includes: ${QTERMWIDGET_INCLUDE_DIR}")
    MESSAGE(STATUS "     libs: ${QTERMWIDGET_LIBRARIES}")
    MESSAGE(STATUS "    share: ${QTERMWIDGET_SHARE}")

ELSE (QTERMWIDGET_FOUND)

    MESSAGE(STATUS "QTermWidget not found.")
    MESSAGE(STATUS "  You can specify includes: -DQTERMWIDGET_PATH_INCLUDES=/some/path")
    MESSAGE(STATUS "  currently found includes: ${QTERMWIDGET_INCLUDE_DIR}")
    MESSAGE(STATUS "      You can specify libs: -DQTERMWIDGET_PATH_LIB=/another/path")
    MESSAGE(STATUS "      currently found libs: ${QTERMWIDGET_LIBRARIES}")
    MESSAGE(STATUS "     You can specify share: -DQTERMWIDGET_PATH_SHARE=/some/path")
    MESSAGE(STATUS "     currently found share: ${QTERMWIDGET_SHARE}")

    IF (QTermWidget_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find QTermWidget library")
    ENDIF (QTermWidget_FIND_REQUIRED)

ENDIF (QTERMWIDGET_FOUND)

