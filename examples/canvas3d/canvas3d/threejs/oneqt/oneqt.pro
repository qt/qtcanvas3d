!include( ../../../../examples.pri ) {
    error( "Couldn't find the examples.pri file!" )
}

TEMPLATE = app

QT += qml quick

SOURCES += main.cpp

OTHER_FILES += doc/src/* \
               doc/images/*

RESOURCES += oneqt.qrc

DISTFILES += \
    ImageCube.qml
