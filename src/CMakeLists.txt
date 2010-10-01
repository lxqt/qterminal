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

set( QTERM_UI_SRC
	forms/qterminal.ui
    forms/propertiesdialog.ui
)

set( QTERM_RCC_SRC
	icons.qrc
)


QT4_WRAP_UI( QTERM_UI ${QTERM_UI_SRC} )
QT4_WRAP_CPP( QTERM_MOC ${QTERM_MOC_SRC} )
QT4_ADD_RESOURCES( QTERM_RCC ${QTERM_RCC_SRC} )

include_directories (
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${QTERMWIDGET_INCLUDE_DIR}
)

# TODO/FIXME: apple bundle
set(GUI_TYPE "")

add_executable( ${EXE_NAME} ${GUI_TYPE}
	${QTERM_SRC}
	${QTERM_UI}
	${QTERM_MOC}
	${QTERM_RCC}
)

target_link_libraries( ${EXE_NAME}
	${QT_LIBRARIES}
	${QTERMWIDGET_LIBRARIES}
)


INSTALL(TARGETS ${EXE_NAME} RUNTIME DESTINATION bin)

