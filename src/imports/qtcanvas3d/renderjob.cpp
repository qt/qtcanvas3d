/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtCanvas3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "renderjob_p.h"
#include "canvasrenderer_p.h"
#include "canvas3d_p.h"

#include <QtGui/QOpenGLContext>

QT_BEGIN_NAMESPACE
QT_CANVAS3D_BEGIN_NAMESPACE

/*!
 * \internal
 *
 * The CanvasRenderJob class is for synchronous render jobs used by QtCanvas3D.
 * It takes care of synchronization.
 */
CanvasRenderJob::CanvasRenderJob(GlSyncCommand *command,
                                 QMutex *mutex, QWaitCondition *condition,
                                 CanvasRenderer *renderer)
    : m_command(command), m_mutex(mutex), m_condition(condition), m_renderer(renderer)
{
}

CanvasRenderJob::~CanvasRenderJob()
{
    notifyGuiThread();
}

void CanvasRenderJob::run()
{
    if (m_renderer && m_renderer->contextCreated()) {
        // Store Qt context
        QOpenGLContext *oldContext = QOpenGLContext::currentContext();
        QSurface *oldSurface = oldContext->surface();

        // Execute pending queue
        m_renderer->transferCommands();
        m_renderer->makeCanvasContextCurrent();
        m_renderer->executeCommandQueue();

        // Execute synchronous command (if there is one)
        if (m_command)
            m_renderer->executeSyncCommand(*m_command);

        // Restore Qt context
        if (oldContext && oldSurface) {
            if (!oldContext->makeCurrent(oldSurface)) {
                qCWarning(canvas3drendering).nospace() << "CanvasRenderJob::" << __FUNCTION__
                                                       << " Failed to make old surface current";
            }
        }
    }

    notifyGuiThread();
}

void CanvasRenderJob::notifyGuiThread()
{
    if (m_mutex) {
        m_mutex->lock();
        m_condition->wakeOne();
        m_mutex->unlock();
        m_mutex = 0;
    }
}

QT_CANVAS3D_END_NAMESPACE
QT_END_NAMESPACE