INCLUDEPATH += ../../../include

LIBS += -L$$OUT_PWD/../../../lib

TEMPLATE = app

QT += qml quick


target.path = $$[QT_INSTALL_EXAMPLES]/canvas3d/$$TARGET
INSTALLS += target
