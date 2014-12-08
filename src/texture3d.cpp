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

#include "texture3d_p.h"

QT_CANVAS3D_BEGIN_NAMESPACE

/*!
 * \qmltype Texture3D
 * \since QtCanvas3D 1.0
 * \ingroup qtcanvas3d-qml-types
 * \brief Contains an OpenGL texture.
 *
 * An uncreatable QML type that contains an OpenGL texture. You can get it by calling
 * \l{Context3D::createTexture()}{Context3D.createTexture()} method.
 */

/*!
 * \internal
 */
CanvasTexture::CanvasTexture(QObject *parent) :
    CanvasAbstractObject(parent),
    m_textureId(0),
    m_isAlive(true)
{
    initializeOpenGLFunctions();
    glGenTextures(1, &m_textureId);
}

/*!
 * \internal
 */
CanvasTexture::~CanvasTexture()
{
    if (m_textureId)
        glDeleteTextures(1, &m_textureId);
}

/*!
 * \internal
 */
void CanvasTexture::bind(CanvasContext::glEnums target)
{
    if (!m_textureId)
        return;

    glBindTexture(GLenum(target), m_textureId);
}

/*!
 * \internal
 */
GLuint CanvasTexture::textureId() const
{
    if (!m_isAlive)
        return 0;

    return m_textureId;
}

/*!
 * \internal
 */
bool CanvasTexture::isAlive() const
{
    return bool(m_textureId);
}

/*!
 * \internal
 */
void CanvasTexture::del()
{
    if (m_textureId)
        glDeleteTextures(1, &m_textureId);
    m_textureId = 0;
}

/*!
 * \internal
 */
QDebug operator<<(QDebug dbg, const CanvasTexture *texture)
{
    if (texture)
        dbg.nospace() << "Texture3D("<< ((void*) texture) << ", name:" << texture->name() << ", id:" << texture->textureId() << ")";
    else
        dbg.nospace() << "Texture3D("<< ((void*) texture) <<")";
    return dbg.maybeSpace();
}

QT_CANVAS3D_END_NAMESPACE
