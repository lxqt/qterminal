cmake_minimum_required(VERSION 3.18.0 FATAL_ERROR)
# CMP0000: Call the cmake_minimum_required() command at the beginning of the top-level
# CMakeLists.txt file even before calling the project() command.
# The cmake_minimum_required(VERSION) command implicitly invokes the cmake_policy(VERSION)
# command to specify that the current project code is written for the given range of CMake
# versions.
project(qterminal)

include(GNUInstallDirs)

# qterminal version
set(QTERMINAL_VERSION "2.2.1")

option(UPDATE_TRANSLATIONS "Update source translation translations/*.ts files" OFF)
option(BUILD_TESTS "Builds tests" ON)

if(APPLE)
    option(APPLEBUNDLE "Build as qterminal.app bundle" ON)
endif()

# we need qpa/qplatformnativeinterface.h for global shortcut

# Minimum Versions
set(LXQTBT_MINIMUM_VERSION "2.2.0")
set(QTERMWIDGET_MINIMUM_VERSION "2.2.0")
set(QT_MINIMUM_VERSION "6.6.0")
set(QT_MAJOR_VERSION "6")
set(SHELLQT_MINIMUM_VERSION "6.0.0")

find_package(Qt6Core ${QT_MINIMUM_VERSION} REQUIRED)
find_package(Qt6Gui ${QT_MINIMUM_VERSION} REQUIRED)
find_package(Qt6LinguistTools ${QT_MINIMUM_VERSION} REQUIRED)
find_package(Qt6Widgets ${QT_MINIMUM_VERSION} REQUIRED)
find_package(LayerShellQt ${SHELLQT_MINIMUM_VERSION} REQUIRED)
if(UNIX)
    find_package(Qt6DBus ${QT_MINIMUM_VERSION} REQUIRED)
    find_package(Qt6 COMPONENTS Core REQUIRED)
endif()
find_package(QTermWidget6 ${QTERMWIDGET_MINIMUM_VERSION} REQUIRED)
find_package(lxqt2-build-tools ${LXQTBT_MINIMUM_VERSION} REQUIRED)

if (BUILD_TESTS)
    find_package(Qt6 ${QT_MINIMUM_VERSION} CONFIG REQUIRED Test)
endif()

include(LXQtPreventInSourceBuilds)
include(FindPkgConfig)
pkg_check_modules(LIBCANBERRA libcanberra)
include(LXQtTranslateTs)
include(LXQtTranslateDesktop)
include(LXQtCompilerSettings NO_POLICY_SCOPE)
message(STATUS "Qt version: ${Qt6Core_VERSION}")

# TODO remove Qxt
message(STATUS "Using bundled Qxt...")
set(QXT_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/third-party")

if(APPLE)
   find_library(CARBON_LIBRARY Carbon REQUIRED)
   message(STATUS "CARBON_LIBRARY: ${CARBON_LIBRARY}")
elseif(UNIX)
   find_package(X11 REQUIRED)
   message(STATUS "X11_X11_LIB: ${X11_X11_LIB}")
endif()

add_definitions(
    -DQTERMINAL_VERSION=\"${QTERMINAL_VERSION}\"
)

set(EXE_NAME qterminal)

set(QTERM_SRC
    src/main.cpp
    src/mainwindow.cpp
    src/tabbar.cpp
    src/tabwidget.cpp
    src/termwidget.cpp
    src/termwidgetholder.cpp
    src/terminalconfig.cpp
    src/properties.cpp
    src/propertiesdialog.cpp
    src/bookmarkswidget.cpp
    src/fontdialog.cpp
    src/dbusaddressable.cpp
    src/tab-switcher.cpp
    src/qterminalutils.cpp
)

set(QTERM_MOC_SRC
    src/qterminalapp.h
    src/mainwindow.h
    src/tabbar.h
    src/tabwidget.h
    src/termwidget.h
    src/termwidgetholder.h
    src/propertiesdialog.h
    src/bookmarkswidget.h
    src/fontdialog.h
    src/tab-switcher.h
)

if (Qt6DBus_FOUND)
    add_definitions(-DHAVE_QDBUS)
    QT6_ADD_DBUS_ADAPTOR(QTERM_SRC src/org.lxqt.QTerminal.Window.xml mainwindow.h MainWindow)
    QT6_ADD_DBUS_ADAPTOR(QTERM_SRC src/org.lxqt.QTerminal.Tab.xml termwidgetholder.h TermWidgetHolder)
    QT6_ADD_DBUS_ADAPTOR(QTERM_SRC src/org.lxqt.QTerminal.Terminal.xml termwidget.h TermWidget)
    QT6_ADD_DBUS_ADAPTOR(QTERM_SRC src/org.lxqt.QTerminal.Process.xml qterminalapp.h QTerminalApp)

    set(QTERM_MOC_SRC ${QTERM_MOC_SRC} src/dbusaddressable.h)
    message(STATUS "Building with Qt6DBus support")
endif()

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

qt6_wrap_ui( QTERM_UI ${QTERM_UI_SRC} )
qt6_wrap_cpp( QTERM_MOC ${QTERM_MOC_SRC} )
qt6_add_resources( QTERM_RCC ${QTERM_RCC_SRC} )

lxqt_translate_ts(QTERM_QM
    UPDATE_TRANSLATIONS
        ${UPDATE_TRANSLATIONS}
    SOURCES
        ${QTERM_SRC}
        ${QTERM_UI_SRC}
        ${QTERM_MOC_SRC}
    TRANSLATION_DIR "src/translations"
)

lxqt_translate_desktop(DESKTOP_FILES
    SOURCES
        qterminal.desktop.in
        qterminal-drop.desktop.in
    TRANSLATION_DIR "src/translations"
    USE_YAML
)

include_directories(
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_BINARY_DIR}"
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
    set(MACOSX_BUNDLE_INFO_STRING "QTerminal ${QTERMINAL_VERSION}")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.qterminal")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "${QTERMINAL_VERSION}")
    set(MACOSX_BUNDLE_BUNDLE_NAME "${EXE_NAME}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${QTERMINAL_VERSION}")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "${QTERMINAL_VERSION}")
    set(MACOSX_BUNDLE_COPYRIGHT "(c) Petr Vanek &lt;petr@yarpen.cz&gt;")

    set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/src/macosx/qterminal.icns"
        PROPERTIES MACOSX_PACKAGE_LOCATION Resources
    )
    # use icon for app bundle to be visible in finder
    set(APPLE_BUNDLE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/macosx/qterminal.icns")
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
    ${DESKTOP_FILES}
)

target_link_libraries(${EXE_NAME}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    qtermwidget6
    LayerShellQtInterface
)
if(QXT_FOUND)
    target_link_libraries(${EXE_NAME} ${QXT_CORE_LIB} ${QXT_GUI_LIB})
endif()

if (Qt6DBus_FOUND)
    target_link_libraries(${EXE_NAME} ${Qt6DBus_LIBRARIES})
endif()

if(APPLE)
    target_link_libraries(${EXE_NAME} ${CARBON_LIBRARY})
endif()

if(X11_FOUND)
    target_link_libraries(${EXE_NAME} ${X11_X11_LIB})
endif()

if(LIBCANBERRA_FOUND)
    add_definitions(-DHAVE_LIBCANBERRA)
    include_directories(${LIBCANBERRA_INCLUDE_DIRS})
    target_link_libraries(${EXE_NAME} ${LIBCANBERRA_LIBRARIES})
endif()

set(APP_DIR "${CMAKE_INSTALL_FULL_DATADIR}/qterminal")
add_definitions(-DAPP_DIR=\"${APP_DIR}\")

install(FILES
    ${DESKTOP_FILES}
    DESTINATION "${CMAKE_INSTALL_DATADIR}/applications"
)

install(FILES
    qterminal.metainfo.xml
    DESTINATION "${CMAKE_INSTALL_DATADIR}/metainfo"
)

install(FILES
    qterminal_bookmarks_example.xml
    DESTINATION ${APP_DIR}
)

if(NOT APPLEBUNDLE)
    install(TARGETS ${EXE_NAME} RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
    install(FILES ${QTERM_QM} DESTINATION ${TRANSLATIONS_DIR})
    install(FILES src/icons/qterminal.png DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/64x64/apps")
else()
    message(STATUS "Building qterminal.app bundle")

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

    install(FILES ${QTERM_QM} DESTINATION ${CMAKE_INSTALL_PREFIX}/${EXE_NAME}.app/Contents/Resources/translations)

    # Workaround https://bugreports.qt.io/browse/QTBUG-63324
    add_custom_command(TARGET ${EXE_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E touch "${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME}.app/Contents/Resources/empty.lproj"
        COMMENT "Creating Resources/empty.lproj")
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
