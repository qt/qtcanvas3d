load(qt_parts)

TEMPLATE = lib
TARGET = qtcanvas3d
QT += qml quick
DEFINES += QTCANVAS3D_LIBRARY
TARGETPATH = QtCanvas3D
IMPORT_VERSION = $$MODULE_VERSION

# Only build qml plugin static if Qt itself is also built static
!contains(QT_CONFIG, static): CONFIG -= static staticlib

include($$PWD/designer/designer.pri)

QMAKE_DOCS = $$PWD/doc/qtcanvas3d.qdocconf

SOURCES += qcanvas3d_plugin.cpp \
    arraybuffer.cpp \
    arraybufferview.cpp \
    float32array.cpp \
    uint8array.cpp \
    uint16array.cpp \
    int8array.cpp \
    int16array.cpp \
    int32array.cpp \
    uint32array.cpp \
    float64array.cpp \
    typedarrayfactory.cpp \
    typedarray.cpp \
    arrayutils.cpp \
    enumtostringmap.cpp \
    uint8clampedarray.cpp \
    abstractobject3d.cpp \
    canvas3d.cpp \
    buffer3d.cpp \
    canvasrendernode.cpp \
    context3d.cpp \
    contextattributes.cpp \
    contextextensions.cpp \
    framebuffer3d.cpp \
    program3d.cpp \
    renderbuffer3d.cpp \
    shader3d.cpp \
    shaderprecisionformat.cpp \
    teximage3d.cpp \
    teximage3dloader.cpp \
    texture3d.cpp \
    uniformlocation.cpp \
    activeinfo3d.cpp

HEADERS += qcanvas3d_plugin.h \
    arraybuffer_p.h \
    arraybufferview_p.h \
    float32array_p.h \
    uint8array_p.h \
    uint16array_p.h \
    int8array_p.h \
    int16array_p.h \
    int32array_p.h \
    uint32array_p.h \
    float64array_p.h \
    typedarrayfactory_p.h \
    typedarray_p.h \
    arrayutils_p.h \
    enumtostringmap_p.h \
    uint8clampedarray_p.h \
    abstractobject3d_p.h \
    canvas3d_p.h \
    buffer3d_p.h \
    canvas3dcommon_p.h \
    canvasrendernode_p.h \
    context3d_p.h \
    contextattributes_p.h \
    contextextensions_p.h \
    framebuffer3d_p.h \
    program3d_p.h \
    renderbuffer3d_p.h \
    shader3d_p.h \
    shaderprecisionformat_p.h \
    teximage3d_p.h \
    teximage3dloader_p.h \
    texture3d_p.h \
    uniformlocation_p.h \
    activeinfo3d_p.h

OTHER_FILES = qmldir \
    doc/* \
    doc/src/* \
    doc/images/* \
    doc/snippets/* \
    plugins.qmltypes \
    designer/* \
    designer/default/*

CONFIG += no_cxx_module

load(qml_plugin)

!android:!ios {
    copy_qmldir.target = $$DESTDIR/qmldir
    copy_qmldir.depends = $$_PRO_FILE_PWD_/qmldir
    copy_qmldir.commands = $(COPY_FILE) \"$$replace(copy_qmldir.depends, /, $$QMAKE_DIR_SEP)\" \"$$replace(copy_qmldir.target, /, $$QMAKE_DIR_SEP)\"
    QMAKE_EXTRA_TARGETS += copy_qmldir
    PRE_TARGETDEPS += $$copy_qmldir.target
}
