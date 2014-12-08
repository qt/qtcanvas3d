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

#include "arraybufferview_p.h"
#include "arraybuffer_p.h"

QT_CANVAS3D_BEGIN_NAMESPACE

/*!
 * \qmltype ArrayBufferView
 * \since QtCanvas3D 1.0
 * \ingroup qtcanvas3d-qml-types
 * \brief Contains an array buffer view.
 *
 * An uncreatable QML type that contains a particular view to the data of an ArrayBuffer.
 */

/*!
 * \internal
 */
CanvasArrayBufferView::CanvasArrayBufferView(CanvasArrayBuffer *buffer, unsigned long byteOffset,
                                             QObject *parent) :
    CanvasAbstractObject(parent),
    m_buffer(buffer),
    m_byteOffset(byteOffset)
{
    if (m_buffer)
        m_buffer->setParent(this);
}

/*!
 * \internal
 */
CanvasArrayBufferView::~CanvasArrayBufferView()
{
    delete m_buffer;
}

/*!
 * \internal
 */
unsigned char* CanvasArrayBufferView::rawDataCptr() const
{
    if (!m_buffer)
        return 0;

    return m_buffer->m_rawData;
}

/*!
 * \qmlproperty int ArrayBufferView::byteOffset()
 * Contains the offset from the start of the ArrayBuffer of the view in bytes.
 */
/*!
 * \internal
 */
unsigned long CanvasArrayBufferView::byteOffset()
{
    if (m_buffer)
        return m_byteOffset;

    return 0;
}

/*!
 * \qmlproperty int ArrayBufferView::byteLength()
 * Contains the length of the view in bytes.
 */
/*!
 * \internal
 */
unsigned long CanvasArrayBufferView::byteLength()
{
    if (m_buffer)
        return m_buffer->byteLength();

    return 0;
}

/*!
 * \qmlproperty ArrayBuffer ArrayBufferView::buffer()
 * Contains the ArrayBuffer of the view.
 */
/*!
 * \internal
 */
CanvasArrayBuffer *CanvasArrayBufferView::buffer()
{
    return m_buffer;
}

QT_CANVAS3D_END_NAMESPACE
