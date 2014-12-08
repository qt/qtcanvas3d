/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtCanvas3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free 
** Software Foundation and appearing in the file LICENSE.GPL included in 
** the packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "typedarrayfactory_p.h"
#include "arraybuffer_p.h"
#include "int8array_p.h"
#include "uint8array_p.h"
#include "int16array_p.h"
#include "uint16array_p.h"
#include "int32array_p.h"
#include "uint32array_p.h"
#include "float32array_p.h"
#include "float64array_p.h"
#include "uint8clampedarray_p.h"
#include "../canvas3dcommon_p.h"
#include "../teximage3d_p.h"

#include <QDebug>

QT_CANVAS3D_BEGIN_NAMESPACE

/*!
 * \qmltype Arrays
 * \since QtCanvas3D 1.0
 * \ingroup qtcanvas3d-qml-types
 * \brief Create typed arrays.
 *
 * This QML type can be used for creating typed arrays of required type.
 *
 * \sa Float32Array, Float64Array, Int8Array, Int16Array, Int32Array, Uint8Array,
 * Uint16Array, Uint32Array, Uint8ClampedArray
 */

/*!
 * \internal
 */
QObject *CanvasTypedArrayFactory::type_array_factory_provider(QQmlEngine *engine,
                                                              QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    return new CanvasTypedArrayFactory();
}

/*!
 * \internal
 */
CanvasTypedArrayFactory::CanvasTypedArrayFactory(QObject *parent) :
    QObject(parent)
{
}

/*!
 * \internal
 */
CanvasTypedArrayFactory::~CanvasTypedArrayFactory()
{
}

/* Typed array creation functions. Typed arrays should be in the V4VM implementation.*/

/*!
 * \qmlmethod Int8Array Arrays::newInt8Array(int length)
 * Returns a new Int8Array of \a length.
 */
/*!
 * \internal
 */
CanvasInt8Array *CanvasTypedArrayFactory::newInt8Array(unsigned long length)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(unsigned long "<<length<<")";
    return new CanvasInt8Array(length);
}

/*!
 * \qmlmethod Int8Array Arrays::newInt8Array(TypedArray array)
 * Returns a new Int8Array with data from \a array.
 */
/*!
 * \internal
 */
CanvasInt8Array *CanvasTypedArrayFactory::newInt8Array(CanvasTypedArray *array)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(TypedArray " << array << ")";
    return new CanvasInt8Array(array);
}

/*!
 * \qmlmethod Int8Array Arrays::newInt8Array(list<variant> array)
 * Returns a new Int8Array with data from \a array.
 */
/*!
 * \internal
 */
CanvasInt8Array *CanvasTypedArrayFactory::newInt8Array(QVariantList array)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(QVariantList" << array.count() << ")";
    return new CanvasInt8Array(&array);
}

/*!
 * \qmlmethod Int8Array Arrays::newInt8Array(ArrayBuffer buffer, int byteOffset)
 * Returns a new Int8Array with data from \a buffer starting from \a byteOffset.
 */
/*!
 * \internal
 */
CanvasInt8Array *CanvasTypedArrayFactory::newInt8Array(CanvasArrayBuffer *buffer,
                                                       unsigned long byteOffset)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(ArrayBuffer buffer, unsigned long byteOffset)";
    return new CanvasInt8Array(buffer, byteOffset);
}

/*!
 * \qmlmethod Uint8Array Arrays::newUint8Array(int length)
 * Returns a new Uint8Array of \a length.
 */
/*!
 * \internal
 */
CanvasUint8Array *CanvasTypedArrayFactory::newUint8Array(unsigned long length)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(unsigned long "<<length<<")";
    return new CanvasUint8Array(length);
}

/*!
 * \qmlmethod Uint8Array Arrays::newUint8Array(TypedArray array)
 * Returns a new Uint8Array with data from \a array.
 */
/*!
 * \internal
 */
CanvasUint8Array *CanvasTypedArrayFactory::newUint8Array(CanvasTypedArray *array)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(TypedArray " << array << ")";
    return new CanvasUint8Array(array);
}

/*!
 * \qmlmethod Uint8Array Arrays::newUint8Array(list<variant> array)
 * Returns a new Uint8Array with data from \a array.
 */
/*!
 * \internal
 */
CanvasUint8Array *CanvasTypedArrayFactory::newUint8Array(QVariantList array)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(QVariantList" << array.count() << ")";
    return new CanvasUint8Array(&array);
}

/*!
 * \qmlmethod Uint8Array Arrays::newUint8Array(ArrayBuffer buffer, int byteOffset)
 * Returns a new Uint8Array with data from \a buffer starting from \a byteOffset.
 */
/*!
 * \internal
 */
CanvasUint8Array *CanvasTypedArrayFactory::newUint8Array(CanvasArrayBuffer *buffer,
                                                         unsigned long byteOffset)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(ArrayBuffer buffer, unsigned long byteOffset)";
    return new CanvasUint8Array(buffer, byteOffset);
}

/*!
 * \qmlmethod Int16Array Arrays::newInt16Array(int length)
 * Returns a new Int16Array of \a length.
 */
/*!
 * \internal
 */
CanvasInt16Array *CanvasTypedArrayFactory::newInt16Array(unsigned long length)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(unsigned long "<<length<<")";
    return new CanvasInt16Array(length);
}

/*!
 * \qmlmethod Int16Array Arrays::newInt16Array(TypedArray array)
 * Returns a new Int16Array with data from \a array.
 */
/*!
 * \internal
 */
CanvasInt16Array *CanvasTypedArrayFactory::newInt16Array(CanvasTypedArray *array)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(TypedArray " << array << ")";
    return new CanvasInt16Array(array);
}

/*!
 * \qmlmethod Int16Array Arrays::newInt16Array(list<variant> array)
 * Returns a new Int16Array with data from \a array.
 */
/*!
 * \internal
 */
CanvasInt16Array *CanvasTypedArrayFactory::newInt16Array(QVariantList array)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(QVariantList" << array.count() << ")";
    return new CanvasInt16Array(&array);
}

/*!
 * \qmlmethod Int16Array Arrays::newInt16Array(ArrayBuffer buffer, int byteOffset)
 * Returns a new Int16Array with data from \a buffer starting from \a byteOffset.
 */
/*!
 * \internal
 */
CanvasInt16Array *CanvasTypedArrayFactory::newInt16Array(CanvasArrayBuffer *buffer,
                                                         unsigned long byteOffset)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(ArrayBuffer buffer, unsigned long byteOffset)";
    return new CanvasInt16Array(buffer, byteOffset);
}

/*!
 * \qmlmethod Uint16Array Arrays::newUint16Array(int length)
 * Returns a new Uint16Array of \a length.
 */
/*!
 * \internal
 */
CanvasUint16Array *CanvasTypedArrayFactory::newUint16Array(unsigned long length)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(unsigned long "<<length<<")";
    return new CanvasUint16Array(length);
}

/*!
 * \qmlmethod Uint16Array Arrays::newUint16Array(TypedArray array)
 * Returns a new Uint16Array with data from \a array.
 */
/*!
 * \internal
 */
CanvasUint16Array *CanvasTypedArrayFactory::newUint16Array(CanvasTypedArray *array)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(TypedArray " << array << ")";
    return new CanvasUint16Array(array);
}

/*!
 * \qmlmethod Uint16Array Arrays::newUint16Array(list<variant> array)
 * Returns a new Uint16Array with data from \a array.
 */
/*!
 * \internal
 */
CanvasUint16Array *CanvasTypedArrayFactory::newUint16Array(QVariantList array)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(QVariantList" << array.count() << ")";
    return new CanvasUint16Array(&array);
}

/*!
 * \qmlmethod Uint16Array Arrays::newUint16Array(ArrayBuffer buffer, int byteOffset)
 * Returns a new Uint16Array with data from \a buffer starting from \a byteOffset.
 */
/*!
 * \internal
 */
CanvasUint16Array *CanvasTypedArrayFactory::newUint16Array(CanvasArrayBuffer *buffer,
                                                           unsigned long byteOffset)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(ArrayBuffer buffer, unsigned long byteOffset)";
    return new CanvasUint16Array(buffer, byteOffset);
}

/*!
 * \qmlmethod Int32Array Arrays::newInt32Array(int length)
 * Returns a new Int32Array of \a length.
 */
/*!
 * \internal
 */
CanvasInt32Array *CanvasTypedArrayFactory::newInt32Array(unsigned long length)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(unsigned long "<<length<<")";
    return new CanvasInt32Array(length);
}

/*!
 * \qmlmethod Int32Array Arrays::newInt32Array(TypedArray array)
 * Returns a new Int32Array with data from \a array.
 */
/*!
 * \internal
 */
CanvasInt32Array *CanvasTypedArrayFactory::newInt32Array(CanvasTypedArray *array)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(TypedArray " << array << ")";
    return new CanvasInt32Array(array);
}

/*!
 * \qmlmethod Int32Array Arrays::newInt32Array(list<variant> array)
 * Returns a new Int32Array with data from \a array.
 */
/*!
 * \internal
 */
CanvasInt32Array *CanvasTypedArrayFactory::newInt32Array(QVariantList array)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(QVariantList" << array.count() << ")";
    return new CanvasInt32Array(&array);
}

/*!
 * \qmlmethod Int32Array Arrays::newInt32Array(ArrayBuffer buffer, int byteOffset)
 * Returns a new Int32Array with data from \a buffer starting from \a byteOffset.
 */
/*!
 * \internal
 */
CanvasInt32Array *CanvasTypedArrayFactory::newInt32Array(CanvasArrayBuffer *buffer,
                                                         unsigned long byteOffset)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(ArrayBuffer buffer, unsigned long byteOffset)";
    return new CanvasInt32Array(buffer, byteOffset);
}

/*!
 * \qmlmethod Uint32Array Arrays::newUint32Array(int length)
 * Returns a new Uint32Array of \a length.
 */
/*!
 * \internal
 */
CanvasUint32Array *CanvasTypedArrayFactory::newUint32Array(unsigned long length)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(unsigned long "<<length<<")";
    return new CanvasUint32Array(length);
}

/*!
 * \qmlmethod Uint32Array Arrays::newUint32Array(TypedArray array)
 * Returns a new Uint32Array with data from \a array.
 */
/*!
 * \internal
 */
CanvasUint32Array *CanvasTypedArrayFactory::newUint32Array(CanvasTypedArray *array)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(TypedArray " << array << ")";
    return new CanvasUint32Array(array);
}

/*!
 * \qmlmethod Uint32Array Arrays::newUint32Array(list<variant> array)
 * Returns a new Uint32Array with data from \a array.
 */
/*!
 * \internal
 */
CanvasUint32Array *CanvasTypedArrayFactory::newUint32Array(QVariantList array)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(QVariantList" << array.count() << ")";
    return new CanvasUint32Array(&array);
}

/*!
 * \qmlmethod Uint32Array Arrays::newUint32Array(ArrayBuffer buffer, int byteOffset)
 * Returns a new Uint32Array with data from \a buffer starting from \a byteOffset.
 */
/*!
 * \internal
 */
CanvasUint32Array *CanvasTypedArrayFactory::newUint32Array(CanvasArrayBuffer *buffer,
                                                           unsigned long byteOffset)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(ArrayBuffer buffer, unsigned long byteOffset)";
    return new CanvasUint32Array(buffer, byteOffset);
}

/*!
 * \qmlmethod Float32Array Arrays::newFloat32Array(int length)
 * Returns a new Float32Array of \a length.
 */
/*!
 * \internal
 */
CanvasFloat32Array *CanvasTypedArrayFactory::newFloat32Array(unsigned long length)
{
    CanvasFloat32Array *newArray = new CanvasFloat32Array(length);
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(unsigned long "<<length<<"):" << newArray;
    return newArray;
}

/*!
 * \qmlmethod Float32Array Arrays::newFloat32Array(TypedArray array)
 * Returns a new Float32Array with data from \a array.
 */
/*!
 * \internal
 */
CanvasFloat32Array *CanvasTypedArrayFactory::newFloat32Array(CanvasTypedArray *array)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(TypedArray " << array << ")";
    return new CanvasFloat32Array(array);
}

/*!
 * \qmlmethod Float32Array Arrays::newFloat32Array(list<variant> array)
 * Returns a new Float32Array with data from \a array.
 */
/*!
 * \internal
 */
CanvasFloat32Array *CanvasTypedArrayFactory::newFloat32Array(QVariantList array)
{
    CanvasFloat32Array *newArray = new CanvasFloat32Array(&array);
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(QVariantList" << array.count() << ")";
    return newArray;
}

/*!
 * \qmlmethod Float32Array Arrays::newFloat32Array(ArrayBuffer buffer, int byteOffset)
 * Returns a new Float32Array with data from \a buffer starting from \a byteOffset.
 */
/*!
 * \internal
 */
CanvasFloat32Array *CanvasTypedArrayFactory::newFloat32Array(CanvasArrayBuffer *buffer,
                                                             unsigned long byteOffset)
{
    CanvasFloat32Array *newArray = new CanvasFloat32Array(buffer, byteOffset);
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(ArrayBuffer "<<buffer<<", unsigned long "<<byteOffset<<"):"<<newArray;
    return newArray;
}

/*!
 * \qmlmethod Float64Array Arrays::newFloat64Array(int length)
 * Returns a new Float64Array of \a length.
 */
/*!
 * \internal
 */
CanvasFloat64Array *CanvasTypedArrayFactory::newFloat64Array(unsigned long length)
{
    CanvasFloat64Array *newArray = new CanvasFloat64Array(length);
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(unsigned long "<<length<<"):" << newArray;
    return newArray;
}

/*!
 * \qmlmethod Float64Array Arrays::newFloat64Array(TypedArray array)
 * Returns a new Float64Array with data from \a array.
 */
/*!
 * \internal
 */
CanvasFloat64Array *CanvasTypedArrayFactory::newFloat64Array(CanvasTypedArray *array)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(TypedArray " << array << ")";
    return new CanvasFloat64Array(array);
}

/*!
 * \qmlmethod Float64Array Arrays::newFloat64Array(list<variant> array)
 * Returns a new Float64Array with data from \a array.
 */
/*!
 * \internal
 */
CanvasFloat64Array *CanvasTypedArrayFactory::newFloat64Array(QVariantList array)
{
    CanvasFloat64Array * newArray = new CanvasFloat64Array(&array);
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(QVariantList" << array.count() << ")";
    return newArray;
}

/*!
 * \qmlmethod Float64Array Arrays::newFloat64Array(ArrayBuffer buffer, int byteOffset)
 * Returns a new Float64Array with data from \a buffer starting from \a byteOffset.
 */
/*!
 * \internal
 */
CanvasFloat64Array *CanvasTypedArrayFactory::newFloat64Array(CanvasArrayBuffer *buffer,
                                                             unsigned long byteOffset)
{
    CanvasFloat64Array *newArray = new CanvasFloat64Array(buffer, byteOffset);
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(ArrayBuffer "<<buffer<<", unsigned long "<<byteOffset<<"):"<<newArray;
    return newArray;
}

/*!
 * \qmlmethod Uint8ClampedArray Arrays::newUint8ClampedArray(int length)
 * Returns a new Uint8ClampedArray of \a length.
 */
/*!
 * \internal
 */
CanvasUint8ClampedArray *CanvasTypedArrayFactory::newUint8ClampedArray(unsigned long length)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(unsigned long "<<length<<")";
    return new CanvasUint8ClampedArray(length);
}

/*!
 * \qmlmethod Uint8ClampedArray Arrays::newUint8ClampedArray(TypedArray array)
 * Returns a new Uint8ClampedArray with data from \a array.
 */
/*!
 * \internal
 */
CanvasUint8ClampedArray *CanvasTypedArrayFactory::newUint8ClampedArray(CanvasTypedArray *array)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(TypedArray " << array << ")";
    return new CanvasUint8ClampedArray(array);
}

/*!
 * \qmlmethod Uint8ClampedArray Arrays::newUint8ClampedArray(list<variant> array)
 * Returns a new Uint8ClampedArray with data from \a array.
 */
/*!
 * \internal
 */
CanvasUint8ClampedArray *CanvasTypedArrayFactory::newUint8ClampedArray(QVariantList array)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(QVariantList" << array.count() << ")";
    return new CanvasUint8ClampedArray(&array);
}

/*!
 * \qmlmethod Uint8ClampedArray Arrays::newUint8ClampedArray(ArrayBuffer buffer, int byteOffset)
 * Returns a new Uint8ClampedArray with data from \a buffer starting from \a byteOffset.
 */
/*!
 * \internal
 */
CanvasUint8ClampedArray *CanvasTypedArrayFactory::newUint8ClampedArray(CanvasArrayBuffer *buffer,
                                                                       unsigned long byteOffset)
{
    if (VERBOSE_ALL_TYPED_ARRAY_CALLS) qDebug() << "Arrays::" << __FUNCTION__ << "(ArrayBuffer buffer, unsigned long byteOffset)";
    return new CanvasUint8ClampedArray(buffer, byteOffset);
}

QT_CANVAS3D_END_NAMESPACE
