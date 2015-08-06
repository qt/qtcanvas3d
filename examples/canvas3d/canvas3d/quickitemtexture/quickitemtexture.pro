!include( ../../../examples.pri ) {
    error( "Couldn't find the examples.pri file!" )
}

SOURCES += main.cpp

RESOURCES += quickitemtexture.qrc

OTHER_FILES += qml/quickitemtexture/* \
               doc/src/* \
               doc/images/*

