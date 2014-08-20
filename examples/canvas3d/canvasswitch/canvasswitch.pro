!include( ../examples.pri ) {
    error( "Couldn't find the examples.pri file!" )
}

SOURCES += main.cpp

OTHER_FILES += qml/canvasswitch/* \
               doc/src/* \
               doc/images/*

RESOURCES += canvasswitch.qrc
