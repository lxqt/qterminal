TARGET = qterminal
TEMPLATE = app

LIBS += -lqtermwidget
DEFINES += STR_VERSION=\\\"1.0\\\"

SOURCES=	src/main.cpp			\
		src/mainwindow.cpp		\
		src/properties.cpp		\
		src/propertiesdialog.cpp	\
		src/tabwidget.cpp		\
		src/termwidget.cpp		\
                src/termwidgetholder.cpp

HEADERS=	src/config.h		\
		src/mainwindow.h	\
		src/propertiesdialog.h	\
		src/properties.h	\
		src/tabwidget.h		\
		src/termwidget.h	\
                src/termwidgetholder.h

INCLUDEPATH+=	src

RESOURCES=	src/icons.qrc

FORMS=	src/forms/propertiesdialog.ui \
        src/forms/qterminal.ui

unix {
	isEmpty(PREFIX) {
		PREFIX = /usr/local
	}
	BINDIR = $$PREFIX/bin

	INSTALLS += target shortcut
	target.path = $$BINDIR

	DATADIR = $$PREFIX/share
	shortcut.path = $$DATADIR/applications
	shortcut.files = qterminal.desktop

}

