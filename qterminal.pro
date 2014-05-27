TARGET = qterminal
TEMPLATE = app
# qt5 only. Please use cmake - it's an official build tool for this software
QT += widgets

CONFIG += link_pkgconfig
PKGCONFIG += qtermwidget5

DEFINES += STR_VERSION=\\\"0.5.0\\\"

SOURCES += $$files(src/*.cpp)
HEADERS += $$files(src/*.h)

INCLUDEPATH += src

RESOURCES += src/icons.qrc
FORMS += $$files(src/forms/*.ui)

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

