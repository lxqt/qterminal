set( EXE_NAME qterminal )

set( QTERM_SRC
    main.cpp
    mainwindow.cpp
    tabwidget.cpp
    termwidget.cpp
    termwidgetholder.cpp
    properties.cpp
    propertiesdialog.cpp
)

set( QTERM_MOC_SRC
    mainwindow.h
    tabwidget.h
    termwidget.h
    termwidgetholder.h
    propertiesdialog.h
)

if (NOT QXT_FOUND)
    set (QTERM_SRC ${QTERM_SRC} third-party/qxtglobalshortcut.cpp )
    set (QTERM_MOC_SRC ${QTERM_MOC_SRC} third-party/qxtglobalshortcut.h )

    if(WIN32)
        set(QTERM_SRC ${QTERM_SRC} third-party/qxtglobalshortcut_win.cpp )
    elseif(APPLE)
        set(QTERM_SRC ${QTERM_SRC} third-party/qxtglobalshortcut_mac.cpp )
    else()
        set(QTERM_SRC ${QTERM_SRC} third-party/qxtglobalshortcut_x11.cpp )
    endif()
endif (NOT QXT_FOUND)

set( QTERM_UI_SRC
    forms/qterminal.ui
    forms/propertiesdialog.ui
)

set( QTERM_RCC_SRC
    icons.qrc
)

set( QTERM_TS
    translations/qterminal_cs.ts
    translations/qterminal_it.ts
    translations/qterminal_ru.ts
)


QT4_WRAP_UI( QTERM_UI ${QTERM_UI_SRC} )
QT4_WRAP_CPP( QTERM_MOC ${QTERM_MOC_SRC} )
QT4_ADD_RESOURCES( QTERM_RCC ${QTERM_RCC_SRC} )
QT4_ADD_TRANSLATION( QTERM_QM ${QTERM_TS} )

include_directories (
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${QTERMWIDGET_INCLUDE_DIR}
)
if (X11_FOUND)
    include_directories(${X11_INCLUDE_DIR})
endif (X11_FOUND)


if (APPLEBUNDLE)
    add_definitions(-DAPPLE_BUNDLE)
else()
    set(TRANSLATIONS_DIR "${CMAKE_INSTALL_PREFIX}/share/qterminal/translations")
    add_definitions(-DTRANSLATIONS_DIR=\"${TRANSLATIONS_DIR}\")
endif()

# TODO/FIXME: apple bundle
set(GUI_TYPE "")
set(APPLE_BUNDLE_SOURCES "")
if (APPLEBUNDLE)
    set(GUI_TYPE MACOSX_BUNDLE)

    # create Info.plist file
    set(MACOSX_BUNDLE_ICON_FILE qterminal.icns)
    set(MACOSX_BUNDLE_INFO_STRING "QTerminal ${STR_VERSION}")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.qterminal")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "${STR_VERSION}")
    set(MACOSX_BUNDLE_BUNDLE_NAME "${EXE_NAME}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${STR_VERSION}")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "${STR_VERSION}")
    set(MACOSX_BUNDLE_COPYRIGHT "(c) Petr Vanek &lt;petr@scribus.info&gt;")

    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/macosx/qterminal.icns
                                PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    # use icon for app bundle to be visible in finder
    set(APPLE_BUNDLE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/macosx/qterminal.icns)

endif (APPLEBUNDLE)

add_executable( ${EXE_NAME} ${GUI_TYPE}
    ${QTERM_SRC}
    ${QTERM_UI}
    ${QTERM_MOC}
    ${QTERM_RCC}
    ${APPLE_BUNDLE_SOURCES}
    ${QTERM_QM}
)

target_link_libraries( ${EXE_NAME}
    ${QT_LIBRARIES}
    ${QTERMWIDGET_LIBRARIES}
    util
)
if (QXT_FOUND)
    target_link_libraries(${EXE_NAME} ${QXT_CORE_LIB} ${QXT_GUI_LIB})
endif (QXT_FOUND)

if (APPLE)
    target_link_libraries(${EXE_NAME} ${CARBON_LIBRARY})
endif (APPLE)
if (X11_FOUND)
    target_link_libraries(${EXE_NAME} ${X11_X11_LIB})
endif (X11_FOUND)


if (NOT APPLEBUNDLE)

    INSTALL(TARGETS ${EXE_NAME} RUNTIME DESTINATION bin)
    INSTALL(FILES ${QTERM_QM} DESTINATION ${TRANSLATIONS_DIR})
    install( FILES icons/qterminal.png DESTINATION share/pixmaps)

else (NOT APPLEBUNDLE)
    message(STATUS "APPLEBUNDLE")

    install(CODE "message(STATUS \"Cleaning previously installed bundle (rm -r)\")")
    install(CODE "execute_process(COMMAND rm -r ${CMAKE_INSTALL_PREFIX}/${EXE_NAME}.app)")

    install( TARGETS ${EXE_NAME} DESTINATION "${CMAKE_INSTALL_PREFIX}" )

    # helper stuff to create real apple bundle.
    # Black magic is summoned here...
    add_subdirectory( macosx )

    # bundle required keytabs from the qtermwidget package as well
    install(CODE "message(STATUS \"Bundling (cp) keytab files from ${QTERMWIDGET_SHARE}/qtermwidget/\")")
    install(CODE "execute_process(COMMAND cp -r ${QTERMWIDGET_SHARE}/qtermwidget/ ${CMAKE_INSTALL_PREFIX}/${EXE_NAME}.app/Contents/Resources)")
    #install(CODE "execute_process(COMMAND ls -l ${CMAKE_INSTALL_PREFIX}/${EXE_NAME}.app/Contents/Resources/kb-layouts/)")

    INSTALL(FILES ${QTERM_QM} DESTINATION ${CMAKE_INSTALL_PREFIX}/${EXE_NAME}.app/Contents/translations)

endif (NOT APPLEBUNDLE)


# make lupdate
# it generates new translation files
add_custom_target(lupdate
    ${QT_QMAKE_EXECUTABLE} -project -o ${CMAKE_CURRENT_BINARY_DIR}/qterminal.pro
    COMMAND ${QT_LUPDATE_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/qterminal.pro
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

