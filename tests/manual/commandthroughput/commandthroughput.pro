!include( ../manualtests.pri ) {
    error( "Couldn't find the manualtests.pri file!" )
}

SOURCES += main.cpp

RESOURCES += commandthroughput.qrc

OTHER_FILES += qml/commandthroughput/*

