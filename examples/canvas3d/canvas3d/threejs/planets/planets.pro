!include( ../../../../examples.pri ) {
    error( "Couldn't find the examples.pri file!" )
}

TEMPLATE = app

QT += qml quick

SOURCES += main.cpp

OTHER_FILES += *.qml \
               planets.js \
               doc/src/* \
               doc/images/*

RESOURCES += planets.qrc
