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

#include "float32array_p.h"
#include "../arrayutils_p.h"
#include "arraybuffer_p.h"

QT_CANVAS3D_BEGIN_NAMESPACE

/*!
 * \qmltype Float32Array
 * \since QtCanvas3D 1.0
 * \ingroup qtcanvas3d-qml-types
 * \brief Contains a typed array of floats.
 *
 * An uncreatable QML type that contains a typed array of floats.
 * Use \l Arrays::newFloat32Array() for getting an array of this type.
 */

/*!
 * \internal
 */
CanvasFloat32Array::CanvasFloat32Array(QObject *parent) :
    CanvasTypedArray(0, 0, parent),
    m_typedBuffer(0)
{
    //qDebug() << "Float32Array(QObject *parent)";
}

/*!
 * \internal
 */
CanvasFloat32Array::CanvasFloat32Array(unsigned long length, QObject *parent) :
    CanvasTypedArray(0, 0, parent),
    m_typedBuffer(0)
{
    setLength(length);
    //qDebug() << "Float32Array(unsigned long length, QObject *parent)";
    if (length > 0) {
        m_buffer = new CanvasArrayBuffer(CanvasTypedArray::length() * sizeof(float));
        m_typedBuffer = (float *)m_buffer->m_rawData;
        for (unsigned long i = 0; i < length; i++)
            m_typedBuffer[i] = 0.0;
    }
}

/*!
 * \internal
 */
CanvasFloat32Array::CanvasFloat32Array(CanvasTypedArray *array, QObject *parent) :
    CanvasTypedArray(0, 0, parent)
{
    setLength(array->length());
    m_buffer = new CanvasArrayBuffer(CanvasTypedArray::length() * bytesPerElement());
    m_typedBuffer = (float *)m_buffer->m_rawData;

    if (!ArrayUtils::copyToTarget<float>(m_typedBuffer, array, CanvasTypedArray::length())) {
        // Conversion failed, make this an empty buffer
        delete m_buffer;
        m_buffer = 0;
        m_typedBuffer = 0;
        setLength(0);
    }
}

/*!
 * \internal
 */
CanvasFloat32Array::CanvasFloat32Array(QVariantList *array, QObject *parent) :
    CanvasTypedArray(0, 0, parent)
{
    //qDebug() << "Float32Array(QVariantList *array[" << array->count() << "], parent:"<< parent <<")";
    setLength(array->count());
    m_buffer = new CanvasArrayBuffer(sizeof(float) * length(), this);
    m_typedBuffer = (float *)m_buffer->m_rawData;
    int idx = 0;
    for (QVariantList::const_iterator it = array->begin(); it != array->end(); ++it) {
        QVariant element = *it;
        if (element.canConvert<float>()) {
            m_typedBuffer[idx] = element.toFloat();
        } else {
            //qDebug() << "Failed conversion to float of "<<element;
            m_typedBuffer[idx] = 0.f;
        }
        idx++;
    }
}

/*!
 * \internal
 */
CanvasFloat32Array::CanvasFloat32Array(CanvasArrayBuffer *buffer, unsigned long byteOffset,
                                       QObject *parent) :
    CanvasTypedArray(buffer, byteOffset, parent)
{
    //qDebug() << "Float32Array(ArrayBuffer *buffer, unsigned long byteOffset, QObject *parent)";
    m_typedBuffer = (float *)(m_buffer->m_rawData + byteOffset);
    setLength((byteLength() - byteOffset) / bytesPerElement());
}

/*!
 * \qmlmethod float Float32Array::operator [] (int index)
 * Returns the array value at \a index.
 */
/*!
 * \internal
 */
float CanvasFloat32Array::operator [] (unsigned long index)
{
    if (index < length())
        return m_typedBuffer[index];
    return 0.0;
}

/*!
 * \qmlmethod float Float32Array::get(int index)
 * Returns the array value at \a index.
 */
/*!
 * \internal
 */
float CanvasFloat32Array::get(unsigned long index)
{
    if (index < length())
        return m_typedBuffer[index];
    return 0.0;
}

/*!
 * \qmlmethod void Float32Array::set(int index, float value)
 * Sets the \a value to \a index in the array.
 */
/*!
 * \internal
 */
void CanvasFloat32Array::set(unsigned long index, float value)
{
    if (index < length()) {
        m_typedBuffer[index] = value;
        return;
    }
    qDebug() << "Float32Array " << this << "invalid index " << index;
}

/*!
 * \qmlmethod void Float32Array::set(TypedArray array, int offset)
 * Copies the \a array to \a offset.
 */
/*!
 * \internal
 */
void CanvasFloat32Array::set(CanvasTypedArray *array, unsigned long offset)
{
    delete m_buffer;

    qDebug() << "Float32Array::set( TypedArray" <<  array << ")";

    setLength(array->length() - offset);
    m_buffer = new CanvasArrayBuffer( (length() - offset) * bytesPerElement());
    m_typedBuffer = (float *)m_buffer->m_rawData;

    // TODO: ADD OFFSET AS PARAMETER TO COPY TARGET!!!!
    if (!ArrayUtils::copyToTarget<float>(m_typedBuffer, array, length() - offset)) {
        // Conversion failed, make this an empty buffer
        delete m_buffer;
        m_buffer = 0;
        m_typedBuffer = 0;
        setLength(0);
    }
}

/*!
 * \qmlmethod void Float32Array::set(list<variant> array, int offset)
 * Copies the \a array to \a offset.
 */
/*!
 * \internal
 */
void CanvasFloat32Array::set(QVariantList array, unsigned long offset)
{
    delete m_buffer;

    qDebug() << "Float32Array::set( QVariantList" <<  array << ")";

    setLength(array.count());
    qDebug() << "Float32Array::set(" << array << ")";
    m_buffer = new CanvasArrayBuffer( (length() - offset) * bytesPerElement());
    m_typedBuffer = (float *)m_buffer->m_rawData;

    ArrayUtils::fillFloatArrayFromVariantList(array, m_typedBuffer);
}


/*
void Float32Array::set(float *array, unsigned long offset)
{

}

TypedArray *Float32Array::subarray(long begin)
{

}

TypedArray *Float32Array::subarray(long begin, long end)
{

}
*/

QDebug operator<<(QDebug dbg, const CanvasFloat32Array *array)
{
    if (array)
        dbg.nospace() << "Float32Array ("<< array->name() <<", length:" << array->length() << ")";
    else
        dbg.nospace() << "Float32Array (null)";
    return dbg.maybeSpace();
}

QT_CANVAS3D_END_NAMESPACE
