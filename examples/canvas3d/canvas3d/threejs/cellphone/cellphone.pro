TEMPLATE = app

QT += qml quick

target.path = $$[QT_INSTALL_EXAMPLES]/canvas3d/$$TARGET
INSTALLS += target

SOURCES += main.cpp

RESOURCES += cellphone.qrc

OTHER_FILES += qml/cellphone/* \
               doc/src/* \
               doc/images/*

