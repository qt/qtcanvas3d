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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtCanvas3D API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef CANVASRENDERER_P_H
#define CANVASRENDERER_P_H

#include "canvas3dcommon_p.h"
#include "contextattributes_p.h"
#include "glcommandqueue_p.h"

#include <QtCore/QElapsedTimer>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QOpenGLFramebufferObject>
#include <QtQuick/QQuickItem>

QT_BEGIN_NAMESPACE

class QOffscreenSurface;
class QQuickWindow;

QT_CANVAS3D_BEGIN_NAMESPACE

class QT_CANVAS3D_EXPORT CanvasRenderer : public QObject, QOpenGLFunctions
{
    Q_OBJECT
    Q_DISABLE_COPY(CanvasRenderer)

public:
    CanvasRenderer(QObject *parent = 0);
    ~CanvasRenderer();

    void createContextShare(QQuickWindow *window, const QSize &initializedSize);

    bool createContext(QQuickWindow *window, const CanvasContextAttributes &contexAttributes,
                       GLint &maxVertexAttribs, QSize &maxSize, int &contextVersion,
                       QSet<QByteArray> &extensions);

    void createFBOs();
    void bindCurrentRenderTarget();
    void setFboSize(const QSize &fboSize);
    QSize fboSize() const { return m_fboSize; }

    uint fps() const { return m_fps; }
    CanvasGlCommandQueue *commandQueue() { return &m_commandQueue; }

    void logGlErrors(const char *funcName);
    void transferCommands();
    void makeCanvasContextCurrent();
    void executeCommandQueue();
    void executeSyncCommand(GlSyncCommand &command);
    void finalizeTexture();

    int maxSamples() const { return m_maxSamples; }
    bool isOpenGLES2() const { return m_isOpenGLES2; }
    bool contextCreated() const { return (m_glContext != 0); }
    bool contextShareCreated() const { return (m_glContextShare != 0); }

    GLuint resolveMSAAFbo();

public slots:
    void shutDown();
    void render();

signals:
    void fpsChanged(uint fps);
    void textureReady(int id, const QSize &size);
    void textureIdResolved(QQuickItem *item);

private:
    QSize m_fboSize;
    QSize m_initializedSize;

    QOpenGLContext *m_glContext;
    QOpenGLContext *m_glContextQt;
    QOpenGLContext *m_glContextShare;
    QQuickWindow *m_contextWindow;

    uint m_fps;
    int m_maxSamples;

    bool m_isOpenGLES2;
    bool m_antialias;
    bool m_preserveDrawingBuffer;

    QOpenGLFramebufferObject *m_antialiasFbo;
    QOpenGLFramebufferObject *m_renderFbo;
    QOpenGLFramebufferObject *m_displayFbo;
    QOpenGLFramebufferObjectFormat m_fboFormat;
    QOpenGLFramebufferObjectFormat m_antialiasFboFormat;
    bool m_recreateFbos;

    QOffscreenSurface *m_offscreenSurface;

    CanvasGlCommandQueue m_commandQueue;
    QVector<GlCommand> m_executeQueue;
    int m_executeQueueCount;
    GLuint m_currentFramebufferId;
    QRect m_forceViewportRect;

    int m_glError;
    QElapsedTimer m_fpsTimer;
    int m_fpsFrames;
};

QT_CANVAS3D_END_NAMESPACE
QT_END_NAMESPACE

#endif // CANVASRENDERER_P_H

