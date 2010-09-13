# - Find QTermWidget
# Find the QTermWidget includes and client library
# This module defines
#  QTERMWIDGET_INCLUDE_DIR, where to find includes
#  QTERMWIDGET_LIBRARIES, the libraries needed to use qtermwidget
#  QTERMWIDGET_FOUND, the flag id the system contains qtermwidget library
#
# Petr Vanek <petr@scribus.info>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if (QTERMWIDGET_INCLUDE_DIR AND QTERMWIDGET_LIBRARIES)
  # Already in cache, be silent
  set(PostgreSQL_FIND_QUIETLY TRUE)
endif (QTERMWIDGET_INCLUDE_DIR AND QTERMWIDGET_LIBRARIES)


find_path(QTERMWIDGET_INCLUDE_DIR qtermwidget.h
   ${QTERMWIDGET_PATH_INCLUDES}/
   /usr/include/
   /usr/local/include/
   /opt/local/include/
)

find_library(QTERMWIDGET_LIBRARIES NAMES qtermwidget libqtermwidget
    PATHS
        ${QTERMWIDGET_PATH_LIB}
        /usr/lib/
	/usr/lib${LIB_SUFFIX}/
	/usr/local/lib/
)


mark_as_advanced(QTERMWIDGET_INCLUDE_DIR QTERMWIDGET_LIBRARIES)

if (QTERMWIDGET_LIBRARIES AND QTERMWIDGET_INCLUDE_DIR)
    set( QTERMWIDGET_FOUND 1 )
endif()


IF (QTERMWIDGET_FOUND)

    MESSAGE(STATUS "QTermWidget found")
    MESSAGE(STATUS " includes: ${QTERMWIDGET_INCLUDE_DIR}")
    MESSAGE(STATUS "     libs: ${QTERMWIDGET_LIBRARIES}")

ELSE (QTERMWIDGET_FOUND)

    MESSAGE(STATUS "QTermWidget not found.")
    MESSAGE(STATUS "  You can specify includes: -DQTERMWIDGET_PATH_INCLUDES=/some/path")
    MESSAGE(STATUS "  currently found includes: ${QTERMWIDGET_INCLUDE_DIR}")
    MESSAGE(STATUS "      You can specify libs: -DQTERMWIDGET_PATH_LIB=/another/path")
    MESSAGE(STATUS "      currently found libs: ${QTERMWIDGET_LIBRARIES}")

    IF (QTermWidget_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find QTermWidget library")
    ENDIF (QTermWidget_FIND_REQUIRED)

ENDIF (QTERMWIDGET_FOUND)

