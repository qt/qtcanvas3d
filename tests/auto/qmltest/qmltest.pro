TEMPLATE = app

TARGET = tst_qmltest

CONFIG += qmltestcase
CONFIG += console

SOURCES += tst_qmltest.cpp

OTHER_FILES += canvas3d/tst_*.qml
DEFINES += QUICK_TEST_SOURCE_DIR=\"\\\"$$PWD/canvas3d\\\"\"
