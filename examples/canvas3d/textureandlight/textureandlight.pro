!include( ../examples.pri ) {
    error( "Couldn't find the examples.pri file!" )
}

SOURCES += main.cpp

OTHER_FILES += qml/textureandlight/* \
               doc/src/* \
               doc/images/*

RESOURCES += textureandlight.qrc
