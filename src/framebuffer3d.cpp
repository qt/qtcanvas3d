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

#include "framebuffer3d_p.h"

QT_CANVAS3D_BEGIN_NAMESPACE

/*!
 * \qmltype FrameBuffer3D
 * \since QtCanvas3D 1.0
 * \ingroup qtcanvas3d-qml-types
 * \brief Contains an OpenGL framebuffer.
 *
 * An uncreatable QML type that contains an OpenGL framebuffer object. You can get it by calling
 * \l{Context3D::createFramebuffer()}{Context3D.createFramebuffer()} method.
 */

/*!
 * \internal
 */
CanvasFrameBuffer::CanvasFrameBuffer(QObject *parent) :
    CanvasAbstractObject(parent),
    m_framebufferId(0)
{
    initializeOpenGLFunctions();
    glGenFramebuffers(1, &m_framebufferId);
}

/*!
 * \internal
 */
CanvasFrameBuffer::~CanvasFrameBuffer()
{
    del();
}

/*!
 * \internal
 */
bool CanvasFrameBuffer::isAlive()
{
    return (m_framebufferId);
}

/*!
 * \internal
 */
void CanvasFrameBuffer::del()
{
    if (m_framebufferId) {
        glDeleteFramebuffers(1, &m_framebufferId);
        m_framebufferId = 0;
    }
}

/*!
 * \internal
 */
GLuint CanvasFrameBuffer::id()
{
    return m_framebufferId;
}

/*!
 * \internal
 */
QDebug operator<<(QDebug dbg, const CanvasFrameBuffer *buffer)
{
    if (buffer)
        dbg.nospace() << "FrameBuffer3D("<< buffer->name() <<", id:" << buffer->m_framebufferId << ")";
    else
        dbg.nospace() << "FrameBuffer3D("<< ((void*) buffer) <<")";
    return dbg.maybeSpace();
}

QT_CANVAS3D_END_NAMESPACE
