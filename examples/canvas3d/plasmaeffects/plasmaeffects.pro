!include( ../examples.pri ) {
    error( "Couldn't find the examples.pri file!" )
}

SOURCES += main.cpp

OTHER_FILES += qml/plasmaeffects/* \
               doc/src/* \
               doc/images/*

RESOURCES += plasmaeffects.qrc
