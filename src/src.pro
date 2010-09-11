
HEADERS += mainwindow.h tabwidget.h config.h colorschemadialog.h
SOURCES += mainwindow.cpp main.cpp tabwidget.cpp colorschemadialog.cpp
FORMS += forms/qterminal.ui forms/colorschemadialog.ui

LIBS += -L ../../qtermwidget -lqtermwidget
INCLUDEPATH += ../../qtermwidget/lib

OBJECTS_DIR = .obj
MOC_DIR = .moc
UI_DIR = .ui

TARGET = ../qterminal

RESOURCES += icons.qrc

target.path = /usr/bin
INSTALLS += target