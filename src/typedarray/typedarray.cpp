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

#include "typedarray_p.h"
#include "arraybuffer_p.h"

QT_CANVAS3D_BEGIN_NAMESPACE

/*!
 * \qmltype TypedArray
 * \since QtCanvas3D 1.0
 * \ingroup qtcanvas3d-qml-types
 * \brief Contains a typed array.
 *
 * An uncreatable QML type that contains a typed array.
 * Use Arrays for creating an array of required type.
 *
 * \sa Arrays, Float32Array, Float64Array, Int8Array, Int16Array, Int32Array, Uint8Array,
 * Uint16Array, Uint32Array, Uint8ClampedArray
 */

/*!
 * \fn virtual inline int CanvasTypedArray::bytesPerElement();
 * \internal
 */

/*!
 * \fn virtual inline CanvasContext3D::glEnums CanvasTypedArray::dataType();
 * \internal
 */

/*!
 * \internal
 */
CanvasTypedArray::CanvasTypedArray(CanvasArrayBuffer *buffer, unsigned long byteOffset,
                                   QObject *parent):
    CanvasArrayBufferView(buffer, byteOffset, parent),
    m_length(0)
{
}

/*!
 * \internal
 */
CanvasTypedArray::~CanvasTypedArray()
{
}

/*!
 * \qmlmethod int TypedArray::length()
 * Holds the length of the array.
 */
/*!
 * \internal
 */
unsigned long CanvasTypedArray::length() const
{
    return m_length;
}

/*!
 * \internal
 */
void CanvasTypedArray::copyFrom(const CanvasArrayBuffer &array)
{
    m_buffer->copyRawDataFrom(array.m_rawData, array.byteLength());
}

/*!
 * \internal
 */
void CanvasTypedArray::setLength(unsigned long length)
{
    if (length == m_length)
        return;

    m_length = length;
    emit lengthChanged(length);
}

/*!
 * \internal
 */
QDebug operator<<(QDebug dbg, const CanvasTypedArray *array)
{
    if (array)
        dbg.nospace() << "TypedArray ("<< array->name() <<", length:" << array->length() << ")";
    else
        dbg.nospace() << "TypedArray (null)";
    return dbg.maybeSpace();
}

QT_CANVAS3D_END_NAMESPACE
