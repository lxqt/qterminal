cmake_minimum_required(VERSION 2.8)

project(qterminal)

include(GNUInstallDirs)

option(USE_SYSTEM_QXT "Use system Qxt Library for global shortcuts" ON)
option(USE_QT5 "Build using Qt5. Default OFF." OFF)

set(STR_VERSION "0.6.0")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

if(USE_QT5 AND USE_SYSTEM_QXT)
    message(FATAL_ERROR "Unable to use system qxt with qt5. Please use -DUSE_SYSTEM_QXT=false.")
endif()

if(USE_QT5)
    find_package(Qt5Widgets REQUIRED)
    find_package(Qt5LinguistTools REQUIRED)
    find_package(QTermWidget5 REQUIRED)
    message(STATUS "Qt5 version: ${Qt5Core_VERSION_STRING}")
else()
    find_package(QTermWidget4 REQUIRED)
endif()

include(${QTERMWIDGET_USE_FILE})

if(USE_SYSTEM_QXT)
    message(STATUS "Using system Qxt...")
    find_package(Qxt REQUIRED COMPONENTS QxtCore QxtGui)
    if(NOT QXT_FOUND)
        message(FATAL_ERROR "System Qxt not found. Install Qxt devel package or use -DUSE_SYSTEM_QXT=0 to build with internal copy of this library")
    endif()

    set(QXT_INCLUDE_DIRS "${QXT_INCLUDE_PATH}/QxtCore" "${QXT_INCLUDE_PATH}/QxtGui")
else()
    message(STATUS "Using bundled Qxt...")
    set(QXT_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/third-party")
endif()


if(APPLE)
   find_library(CARBON_LIBRARY Carbon REQUIRED)
   message(STATUS "CARBON_LIBRARY: ${CARBON_LIBRARY}")
elseif(UNIX)
   find_package(X11 REQUIRED)
   message(STATUS "X11_X11_LIB: ${X11_X11_LIB}")
endif ()

add_definitions(-DSTR_VERSION=\"${STR_VERSION}\")


set(EXE_NAME qterminal)

set(QTERM_SRC
    src/main.cpp
    src/mainwindow.cpp
    src/tabwidget.cpp
    src/termwidget.cpp
    src/termwidgetholder.cpp
    src/properties.cpp
    src/propertiesdialog.cpp
    src/bookmarkswidget.cpp
    src/fontdialog.cpp
)

set(QTERM_MOC_SRC
    src/mainwindow.h
    src/tabwidget.h
    src/termwidget.h
    src/termwidgetholder.h
    src/propertiesdialog.h
    src/bookmarkswidget.h
    src/fontdialog.h
)

if(NOT QXT_FOUND)
    set(QTERM_SRC ${QTERM_SRC} src/third-party/qxtglobalshortcut.cpp)
    set(QTERM_MOC_SRC ${QTERM_MOC_SRC} src/third-party/qxtglobalshortcut.h)

    if(WIN32)
        set(QTERM_SRC ${QTERM_SRC} src/third-party/qxtglobalshortcut_win.cpp)
    elseif(APPLE)
        set(QTERM_SRC ${QTERM_SRC} src/third-party/qxtglobalshortcut_mac.cpp)
    else()
        set(QTERM_SRC ${QTERM_SRC} src/third-party/qxtglobalshortcut_x11.cpp)
    endif()
endif()

set(QTERM_UI_SRC
    src/forms/qterminal.ui
    src/forms/propertiesdialog.ui
    src/forms/bookmarkswidget.ui
    src/forms/fontdialog.ui
)

set(QTERM_RCC_SRC
    src/icons.qrc
)

set(QTERM_TS
    src/translations/qterminal_cs.ts
    src/translations/qterminal_de.ts
    src/translations/qterminal_es.ts
    src/translations/qterminal_et.ts
    src/translations/qterminal_it.ts
    src/translations/qterminal_pt.ts
    src/translations/qterminal_hu.ts
	src/translations/qterminal_ru.ts
)

if(USE_QT5)
    qt5_wrap_ui( QTERM_UI ${QTERM_UI_SRC} )
    qt5_wrap_cpp( QTERM_MOC ${QTERM_MOC_SRC} )
    qt5_add_resources( QTERM_RCC ${QTERM_RCC_SRC} )
    qt5_add_translation( QTERM_QM ${QTERM_TS} )
    # bundled qt5 version of libqxt requires private headers (as for 2014-05-27)
    include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})
else()
    qt4_wrap_ui(QTERM_UI ${QTERM_UI_SRC})
    qt4_wrap_cpp(QTERM_MOC ${QTERM_MOC_SRC})
    qt4_add_resources(QTERM_RCC ${QTERM_RCC_SRC})
    qt4_add_translation(QTERM_QM ${QTERM_TS})
endif()

include_directories (
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_BINARY_DIR}"
    ${QTERMWIDGET_INCLUDE_DIRS}
    ${QXT_INCLUDE_DIRS}
)
if(X11_FOUND)
    include_directories("${X11_INCLUDE_DIR}")
endif()


# TODO/FIXME: apple bundle
set(GUI_TYPE "")
set(APPLE_BUNDLE_SOURCES "")
if(APPLEBUNDLE)
    add_definitions(-DAPPLE_BUNDLE)
    set(GUI_TYPE MACOSX_BUNDLE)

    # create Info.plist file
    set(MACOSX_BUNDLE_ICON_FILE qterminal.icns)
    set(MACOSX_BUNDLE_INFO_STRING "QTerminal ${STR_VERSION}")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.qterminal")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "${STR_VERSION}")
    set(MACOSX_BUNDLE_BUNDLE_NAME "${EXE_NAME}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${STR_VERSION}")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "${STR_VERSION}")
    set(MACOSX_BUNDLE_COPYRIGHT "(c) Petr Vanek &lt;petr@yarpen.cz&gt;")

    set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/macosx/qterminal.icns"
        PROPERTIES MACOSX_PACKAGE_LOCATION Resources
    )
    # use icon for app bundle to be visible in finder
    set(APPLE_BUNDLE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/macosx/qterminal.icns")
else()
    set(TRANSLATIONS_DIR "${CMAKE_INSTALL_FULL_DATADIR}/qterminal/translations")
    add_definitions(-DTRANSLATIONS_DIR=\"${TRANSLATIONS_DIR}\")
endif()

add_executable(${EXE_NAME} ${GUI_TYPE}
    ${QTERM_SRC}
    ${QTERM_UI}
    ${QTERM_MOC}
    ${QTERM_RCC}
    ${APPLE_BUNDLE_SOURCES}
    ${QTERM_QM}
)
target_link_libraries(${EXE_NAME}
    ${QTERMWIDGET_QT_LIBRARIES}
    ${QTERMWIDGET_LIBRARIES}
    util
)
if(QXT_FOUND)
    target_link_libraries(${EXE_NAME} ${QXT_CORE_LIB} ${QXT_GUI_LIB})
endif()

if(APPLE)
    target_link_libraries(${EXE_NAME} ${CARBON_LIBRARY})
endif()

if(X11_FOUND)
    target_link_libraries(${EXE_NAME} ${X11_X11_LIB})
endif()


install(FILES
    qterminal.desktop
    qterminal_drop.desktop
    DESTINATION "${CMAKE_INSTALL_DATADIR}/applications"
)

install(FILES
    qterminal.appdata.xml
    DESTINATION "${CMAKE_INSTALL_DATADIR}/appdata"
)

if(NOT APPLEBUNDLE)
    install(TARGETS ${EXE_NAME} RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
    install(FILES ${QTERM_QM} DESTINATION ${TRANSLATIONS_DIR})
    install(FILES src/icons/qterminal.png DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/64x64/apps")
else()
    message(STATUS "APPLEBUNDLE")

    install(CODE "message(STATUS \"Cleaning previously installed bundle (rm -r)\")")
    install(CODE "execute_process(COMMAND rm -r ${CMAKE_INSTALL_PREFIX}/${EXE_NAME}.app)")

    install(TARGETS ${EXE_NAME} DESTINATION "${CMAKE_INSTALL_PREFIX}")

    # helper stuff to create real apple bundle.
    # Black magic is summoned here...
    if(APPLEBUNDLE_STANDALONE)
        message(STATUS "APPLEBUNDLE_STANDALONE")
        configure_file("${CMAKE_SOURCE_DIR}/bundle.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/bundle.cmake" @ONLY)
        install(SCRIPT "${CMAKE_SOURCE_DIR}/bundle.cmake")
    endif()

    # bundle required keytabs from the qtermwidget package as well
    install(CODE "message(STATUS \"Bundling (cp) keytab files from ${QTERMWIDGET_SHARE}/qtermwidget/\")")
    install(CODE "execute_process(COMMAND cp -r ${QTERMWIDGET_SHARE}/qtermwidget/ ${CMAKE_INSTALL_PREFIX}/${EXE_NAME}.app/Contents/Resources)")

    install(FILES ${QTERM_QM} DESTINATION ${CMAKE_INSTALL_PREFIX}/${EXE_NAME}.app/Contents/translations)
endif()


# make lupdate
# it generates new translation files
add_custom_target(lupdate
    ${QT_QMAKE_EXECUTABLE} -project -o "${CMAKE_CURRENT_BINARY_DIR}/qterminal.pro"
    COMMAND ${QT_LUPDATE_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/qterminal.pro"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

# make dist custom target
set(CPACK_PACKAGE_NAME "qterminal")
set(CPACK_PACKAGE_VERSION ${STR_VERSION})
set(CPACK_SOURCE_GENERATOR "TGZ;TBZ2")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_IGNORE_FILES "/CVS/;/\\\\.svn/;/\\\\.git/;\\\\.swp$;\\\\.#;/#;\\\\.tar.gz$;/CMakeFiles/;CMakeCache.txt;\\\\.qm$;/build/;\\\\.diff$;.DS_Store'")
set(CPACK_SOURCE_IGNORE_FILES ${CPACK_IGNORE_FILES})
include(CPack)
# simulate autotools' "make dist"
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
