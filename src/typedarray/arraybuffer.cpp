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

#include "arraybuffer_p.h"

QT_CANVAS3D_BEGIN_NAMESPACE

/*!
 * \qmltype ArrayBuffer
 * \since QtCanvas3D 1.0
 * \ingroup qtcanvas3d-qml-types
 * \brief Contains an array buffer.
 *
 * An uncreatable QML type that contains an array buffer.
 */

/*!
 * \internal
 */
CanvasArrayBuffer::CanvasArrayBuffer(QObject *parent) :
    QObject(parent),
    m_length(0),
    m_rawData(0)
{
}

/*!
 * \internal
 */
CanvasArrayBuffer::CanvasArrayBuffer(unsigned long length, QObject *parent) :
    QObject(parent),
    m_length(length),
    m_rawData(0)
{
    if (length > 0) {
        m_rawData = new unsigned char[length];
        for (unsigned long i = 0; i < length; i++)
            m_rawData[i] = 0;
    }
    emit byteLengthChanged(length);
}

/*!
 * \internal
 */
CanvasArrayBuffer::~CanvasArrayBuffer()
{
    delete[] m_rawData;
}

/*!
 * \internal
 */
unsigned char *CanvasArrayBuffer::rawData()
{
    return m_rawData;
}

/*!
 * \internal
 */
void CanvasArrayBuffer::copyRawDataFrom(unsigned char *rawData, unsigned long byteLength)
{
    if (m_length != byteLength) {
        delete m_rawData;
        m_rawData = 0;
        m_length = 0;
    }

    m_rawData = new unsigned char[byteLength];
    m_length = byteLength;

    for (unsigned long i = 0; i < byteLength; i++)
        m_rawData[i] = rawData[i];
}

/*!
 * \qmlproperty int ArrayBuffer::byteLength()
 * Contains the length of the array in bytes.
 */
/*!
 * \internal
 */
unsigned long CanvasArrayBuffer::byteLength() const
{
    return m_length;
}

/*!
 * \qmlmethod ArrayBuffer ArrayBuffer::slice(int begin)
 * Returns a new ArrayBuffer that is a copy of this ArrayBuffer's bytes from the \a begin to the
 * end of the buffer.
 */
/*!
 * \internal
 */
CanvasArrayBuffer *CanvasArrayBuffer::slice(long begin)
{
    return slice(begin, m_length);
}

/*!
 * \qmlmethod ArrayBuffer ArrayBuffer::slice(int begin, int end)
 * Returns a new ArrayBuffer that is a copy of this ArrayBuffer's bytes from the \a begin to the
 * \a end.
 */
/*!
 * \internal
 */
CanvasArrayBuffer *CanvasArrayBuffer::slice(long begin, long end)
{
    if (begin < 0)
        begin = m_length + begin - 1;
    if (end < 0)
        end = m_length + end - 1;
    long length = end - begin;

    if (length <= 0)
        return new CanvasArrayBuffer(0, 0);

    CanvasArrayBuffer *newBuffer = new CanvasArrayBuffer(length);
    for (long i = 0; i < length; i++)
        newBuffer->m_rawData[i] = m_rawData[i + begin];
    return newBuffer;
}

/*!
 * \qmlmethod bool ArrayBuffer::isView(Object value)
 * Returns \c true if \a value is ArrayBufferView and \c false otherwise.
 */
/*!
 * \internal
 */
bool CanvasArrayBuffer::isView(const QObject &value)
{
    QString className = QString(value.metaObject()->className());
    return (className == "CanvasInt8Array" ||
            className == "CanvasUint8Array" ||
            className == "CanvasUint8ClampedArray" ||
            className == "CanvasInt16Array" ||
            className == "CanvasUint16Array" ||
            className == "CanvasInt32Array" ||
            className == "CanvasUint32Array" ||
            className == "CanvasFloat32Array" ||
            className == "CanvasFloat64Array");
}

QT_CANVAS3D_END_NAMESPACE
