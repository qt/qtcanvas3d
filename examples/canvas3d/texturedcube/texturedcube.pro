!include( ../examples.pri ) {
    error( "Couldn't find the examples.pri file!" )
}

SOURCES += main.cpp

OTHER_FILES += qml/texturedcube/* \
               doc/src/* \
               doc/images/*

RESOURCES += texturedcube.qrc
