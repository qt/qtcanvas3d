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

#include "int16array_p.h"
#include "../arrayutils_p.h"
#include "arraybuffer_p.h"

QT_CANVAS3D_BEGIN_NAMESPACE

/*!
 * \qmltype Int16Array
 * \since QtCanvas3D 1.0
 * \ingroup qtcanvas3d-qml-types
 * \brief Contains a typed array of 16 bit ints.
 *
 * An uncreatable QML type that contains a typed array of 16 bit ints.
 * Use \l Arrays::newInt16Array() for getting an array of this type.
 */

/*!
 * \internal
 */
CanvasInt16Array::CanvasInt16Array(QObject *parent) :
    CanvasTypedArray(0, 0, parent),
    m_typedBuffer(0)
{
    //qDebug() << "Int16Array(QObject *parent)";
    setLength(0);
}

/*!
 * \internal
 */
CanvasInt16Array::CanvasInt16Array(unsigned long length, QObject *parent) :
    CanvasTypedArray(0, 0, parent),
    m_typedBuffer(0)
{
    //qDebug() << "Int16Array(unsigned long length, QObject *parent)";
    setLength(length);
    if (length > 0) {
        m_buffer = new CanvasArrayBuffer(sizeof(float) * CanvasTypedArray::length());
        m_typedBuffer = (short *)m_buffer->m_rawData;
    }
}

/*!
 * \internal
 */
CanvasInt16Array::CanvasInt16Array(CanvasTypedArray *array, QObject *parent) :
    CanvasTypedArray(0, 0, parent)
{
    setLength(array->length());
    m_buffer = new CanvasArrayBuffer(CanvasTypedArray::length() * this->bytesPerElement());
    m_typedBuffer = (short *)m_buffer->m_rawData;

    if (!ArrayUtils::copyToTarget<short>(m_typedBuffer, array, CanvasTypedArray::length())) {
        // Conversion failed, make this an empty buffer
        delete m_buffer;
        setLength(0);
    }
}

/*!
 * \internal
 */
CanvasInt16Array::CanvasInt16Array(QVariantList *array, QObject *parent) :
    CanvasTypedArray(0, 0, parent)
{
    //qDebug() << "Int16Array(const QVariantList *array[" << array->count() << "], QObject *parent)";
    setLength(array->count());
    m_buffer = new CanvasArrayBuffer(sizeof(short) * CanvasTypedArray::length(), this);
    m_typedBuffer = (short *)m_buffer->m_rawData;
    int idx = 0;
    for (QVariantList::const_iterator it = array->begin(); it != array->end(); ++it) {
        QVariant element = *it;
        if (element.canConvert<short>()) {
            m_typedBuffer[idx] = (short)(element.toInt());
        } else {
            //qDebug() << "Failed conversion to unsigned byte of "<<element;
            m_typedBuffer[idx] = (short)0;
        }
        idx++;
    }
}

/*!
 * \internal
 */
CanvasInt16Array::CanvasInt16Array(CanvasArrayBuffer *buffer, unsigned long byteOffset,
                                   QObject *parent) :
    CanvasTypedArray(buffer, byteOffset, parent)
{
    //qDebug() << "Int16Array(ArrayBuffer *buffer, unsigned long byteOffset, QObject *parent)";
    m_typedBuffer = (short *)(m_buffer->m_rawData + byteOffset);
    setLength((byteLength() - byteOffset) / bytesPerElement());
}

/*!
 * \qmlmethod int Int16Array::operator [] (int index)
 * Returns the array value at \a index.
 */
/*!
 * \internal
 */
short CanvasInt16Array::operator [] (unsigned long index)
{
    if (index < length())
        return m_typedBuffer[index];
    return short(0);
}


/*!
 * \qmlmethod int Int16Array::get(int index)
 * Returns the array value at \a index.
 */
/*!
 * \internal
 */
short CanvasInt16Array::get(unsigned long index)
{
    return m_typedBuffer[index];
}

/*!
 * \qmlmethod void Int16Array::set(int index, int value)
 * Sets the \a value to \a index in the array.
 */
/*!
 * \internal
 */
void CanvasInt16Array::set(unsigned long index, short value)
{
    m_typedBuffer[index] = value;
}

/*
void Int16Array::set(TypedArray *array, unsigned long offset)
{

}

void Int16Array::set(float *array, unsigned long offset)
{

}

TypedArray *Int16Array::subarray(long begin)
{

}

TypedArray *Int16Array::subarray(long begin, long end)
{

}
*/

QT_CANVAS3D_END_NAMESPACE
