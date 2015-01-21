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

#include "uint32array_p.h"
#include "../arrayutils_p.h"
#include "arraybuffer_p.h"

QT_CANVAS3D_BEGIN_NAMESPACE

/*!
 * \qmltype Uint32Array
 * \since QtCanvas3D 1.0
 * \ingroup qtcanvas3d-qml-types
 * \brief Contains a typed array of unsigned ints.
 *
 * An uncreatable QML type that contains a typed array of unsigned ints.
 * Use \l Arrays::newUint32Array() for getting an array of this type.
 */

/*!
 * \internal
 */
CanvasUint32Array::CanvasUint32Array(QObject *parent) :
    CanvasTypedArray(0, 0, parent),
    m_typedBuffer(0)
{
    //qDebug() << "Uint32Array(QObject *parent)";
    setLength(0);
}

/*!
 * \internal
 */
CanvasUint32Array::CanvasUint32Array(unsigned long length, QObject *parent) :
    CanvasTypedArray(0, 0, parent),
    m_typedBuffer(0)
{
    //qDebug() << "Uint32Array(unsigned long length, QObject *parent)";
    setLength(length);
    if (length > 0) {
        m_buffer = new CanvasArrayBuffer(sizeof(float) * CanvasTypedArray::length());
        m_typedBuffer = (unsigned int *)m_buffer->m_rawData;
    }
}

/*!
 * \internal
 */
CanvasUint32Array::CanvasUint32Array(CanvasTypedArray *array, QObject *parent) :
    CanvasTypedArray(0, 0, parent)
{
    setLength(array->length());
    m_buffer = new CanvasArrayBuffer(CanvasTypedArray::length() * this->bytesPerElement());
    m_typedBuffer = (unsigned int *)m_buffer->m_rawData;

    if (!ArrayUtils::copyToTarget<unsigned int>(m_typedBuffer, array, CanvasTypedArray::length())) {
        // Conversion failed, make this an empty buffer
        delete m_buffer;
        setLength(0);
    }
}

/*!
 * \internal
 */
CanvasUint32Array::CanvasUint32Array(QVariantList *array, QObject *parent) :
    CanvasTypedArray(0, 0, parent)
{
    //qDebug() << "Uint32Array(const QVariantList *array[" << array->count() << "], QObject *parent)";
    setLength(array->count());
    m_buffer = new CanvasArrayBuffer(sizeof(unsigned int) * CanvasTypedArray::length(), this);
    m_typedBuffer = (unsigned int *)m_buffer->m_rawData;
    int idx = 0;
    for (QVariantList::const_iterator it = array->begin(); it != array->end(); ++it) {
        QVariant element = *it;
        if (element.canConvert<unsigned int>()) {
            m_typedBuffer[idx] = (unsigned int)(element.toInt());
        } else {
            //qDebug() << "Failed conversion to unsigned byte of "<<element;
            m_typedBuffer[idx] = (unsigned int)0;
        }
        idx++;
    }
}

/*!
 * \internal
 */
CanvasUint32Array::CanvasUint32Array(CanvasArrayBuffer *buffer, unsigned long byteOffset,
                                     QObject *parent) :
    CanvasTypedArray(buffer, byteOffset, parent)
{
    //qDebug() << "Uint32Array(ArrayBuffer *buffer, unsigned long byteOffset, QObject *parent)";
    m_typedBuffer = (unsigned int *)(m_buffer->m_rawData + byteOffset);
    setLength((byteLength() - byteOffset) / bytesPerElement());
}

/*!
 * \qmlmethod int Uint32Array::operator [] (int index)
 */
/*!
 * \internal
 */
unsigned int CanvasUint32Array::operator [] (unsigned long index)
{
    if (index < length())
        return m_typedBuffer[index];
    return uint(0);
}

/*!
 * \qmlmethod int Uint32Array::get(int index)
 */
/*!
 * \internal
 */
unsigned int CanvasUint32Array::get(unsigned long index)
{
    return m_typedBuffer[index];
}

/*!
 * \qmlmethod void Uint32Array::set(int index, int value)
 */
/*!
 * \internal
 */
void CanvasUint32Array::set(unsigned long index, unsigned int value)
{
    m_typedBuffer[index] = value;
}

/*
void Uint32Array::set(TypedArray *array, unsigned long offset)
{

}

void Uint32Array::set(float *array, unsigned long offset)
{

}

TypedArray *Uint32Array::subarray(long begin)
{

}

TypedArray *Uint32Array::subarray(long begin, long end)
{

}
*/

QT_CANVAS3D_END_NAMESPACE
