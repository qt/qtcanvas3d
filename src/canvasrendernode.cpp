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

#include "canvasrendernode_p.h"
#include "canvas3d_p.h"
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFramebufferObject>

QT_CANVAS3D_BEGIN_NAMESPACE

CanvasRenderNode::CanvasRenderNode(Canvas *canvas, QQuickWindow *window) :
    QObject(),
    QSGSimpleTextureNode(),
    m_canvas(canvas),
    m_id(0),
    m_size(0,0),
    m_texture(0),
    m_window(window)
{
    if (m_canvas->logAllCalls()) qDebug() << "CanvasRenderNode::" << __FUNCTION__;

    // Our texture node must have a texture, so use the default 0 texture.
    m_texture = m_window->createTextureFromId(0, QSize(1, 1));
    setTexture(m_texture);
    setFiltering(QSGTexture::Linear);
    setTextureCoordinatesTransform(QSGSimpleTextureNode::MirrorVertically);
}

CanvasRenderNode::~CanvasRenderNode()
{
    delete m_texture;
}

void CanvasRenderNode::newTexture(int id, const QSize &size)
{
    if (m_canvas->logAllCalls()) qDebug() << "CanvasRenderNode::" << __FUNCTION__ << "(" << id << ", " << size << ")";
    m_mutex.lock();
    m_id = id;
    m_size = size;
    m_mutex.unlock();

    // We cannot call QQuickWindow::update directly here, as this is only allowed
    // from the rendering thread or GUI thread.
    emit pendingNewTexture();
}

// Before the scene graph starts to render, we update to the pending texture
void CanvasRenderNode::prepareNode()
{
    if (m_canvas->logAllCalls()) qDebug() << "CanvasRenderNode::" << __FUNCTION__;
    m_mutex.lock();
    int newId = m_id;
    QSize size = m_size;
    m_id = 0;
    m_mutex.unlock();

    if (newId) {
        if (m_canvas->logAllCalls()) qDebug() << "CanvasRenderNode::" << __FUNCTION__ << " showing new texture:" << newId << " size:" << size << " targetRect:" << rect();
        delete m_texture;
        m_texture = m_window->createTextureFromId(newId, size);
        setTexture(m_texture);
        if (m_canvas->logAllCalls()) qDebug() << "CanvasRenderNode::" << __FUNCTION__ << " SGTexture size:" << m_texture->textureSize() << " normalizedTextureSubRect:" << m_texture->normalizedTextureSubRect();

        // This will notify the main thread that the texture is now being rendered
        // and it can start rendering to the other one.
        emit textureInUse();
    } else {
        if (m_canvas->logAllCalls()) qDebug() << "CanvasRenderNode::" << __FUNCTION__ << " showing previous texture";
    }
}

QT_CANVAS3D_END_NAMESPACE
