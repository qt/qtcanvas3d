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

SOURCES += typedarray/arraybuffer.cpp \
    typedarray/arraybufferview.cpp \
    typedarray/float32array.cpp \
    typedarray/uint8array.cpp \
    typedarray/uint16array.cpp \
    typedarray/int8array.cpp \
    typedarray/int16array.cpp \
    typedarray/int32array.cpp \
    typedarray/uint32array.cpp \
    typedarray/float64array.cpp \
    typedarray/typedarrayfactory.cpp \
    typedarray/typedarray.cpp \
    typedarray/uint8clampedarray.cpp \
    qcanvas3d_plugin.cpp \
    arrayutils.cpp \
    enumtostringmap.cpp \
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
    texture3d.cpp \
    uniformlocation.cpp \
    activeinfo3d.cpp \
    canvasglstatedump.cpp

HEADERS += typedarray/arraybuffer_p.h \
    typedarray/arraybufferview_p.h \
    typedarray/float32array_p.h \
    typedarray/uint8array_p.h \
    typedarray/uint16array_p.h \
    typedarray/int8array_p.h \
    typedarray/int16array_p.h \
    typedarray/int32array_p.h \
    typedarray/uint32array_p.h \
    typedarray/float64array_p.h \
    typedarray/typedarrayfactory_p.h \
    typedarray/typedarray_p.h \
    typedarray/arrayutils_p.h \
    typedarray/uint8clampedarray_p.h \
    qcanvas3d_plugin.h \
    enumtostringmap_p.h \
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
    texture3d_p.h \
    uniformlocation_p.h \
    activeinfo3d_p.h \
    canvasglstatedump_p.h

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
