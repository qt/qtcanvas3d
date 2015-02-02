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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtCanvas3D API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef CANVAS3D_P_H
#define CANVAS3D_P_H

#include "canvas3dcommon_p.h"
#include "context3d_p.h"

#include <QQuickItem>
#include <QQuickWindow>

class QOffscreenSurface;

QT_CANVAS3D_BEGIN_NAMESPACE

class QT_CANVAS3D_EXPORT Canvas : public QQuickItem, QOpenGLFunctions
{
    Q_OBJECT
    Q_DISABLE_COPY(Canvas)
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(CanvasContext *context READ context NOTIFY contextChanged)
    Q_PROPERTY(bool logAllCalls READ logAllCalls WRITE setLogAllCalls NOTIFY logAllCallsChanged)
    Q_PROPERTY(bool logAllErrors READ logAllErrors WRITE setLogAllErrors NOTIFY logAllErrorsChanged)
    Q_PROPERTY(float devicePixelRatio READ devicePixelRatio NOTIFY devicePixelRatioChanged)
    Q_PROPERTY(uint fps READ fps NOTIFY fpsChanged)

public:
    Canvas(QQuickItem *parent = 0);
    ~Canvas();

    void handleWindowChanged(QQuickWindow *win);
    float devicePixelRatio();

    void bindCurrentRenderTarget();

    void setLogAllCalls(bool logCalls);
    bool logAllCalls() const;
    void setLogAllErrors(bool logErrors);
    bool logAllErrors() const;

    uint fps();

    Q_INVOKABLE CanvasContext *getContext(const QString &name);
    Q_INVOKABLE CanvasContext *getContext(const QString &name, const QVariantMap &options);
    CanvasContext *context();

public slots:
    void ready();
    void shutDown();
    void renderNext();

signals:
    void needRender();
    void devicePixelRatioChanged(float ratio);
    void animatedChanged(bool animated);
    void logAllCallsChanged(bool logCalls);
    void logAllErrorsChanged(bool logErrors);
    void contextChanged(CanvasContext *context);
    void fpsChanged(uint fps);

    void initGL();
    void renderGL();

    void textureReady(int id, const QSize &size, float devicePixelRatio);

protected:
    virtual void geometryChanged(const QRectF & newGeometry, const QRectF & oldGeometry);
    virtual void itemChange(ItemChange change, const ItemChangeData &value);
    virtual QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *);

private:
    void setupAntialiasing();
    void updateWindowParameters();

    bool m_renderNodeReady;
    bool m_logAllCalls;
    bool m_logAllErrors;
    QThread *m_mainThread;
    QThread *m_contextThread;
    QRectF m_cachedGeometry;
    CanvasContext *m_context3D;
    bool m_isFirstRender;
    QSize m_initialisedSize;

    QOpenGLContext *m_glContext;
    QOpenGLContext *m_glContextQt;
    QQuickWindow *m_contextWindow;

    uint m_fps;
    int m_maxSamples;
    float m_devicePixelRatio;

    bool m_isOpenGLES2;
    bool m_isSoftwareRendered;
    bool m_runningInDesigner;
    CanvasContextAttributes m_contextAttribs;
    bool m_isContextAttribsSet;

    QOpenGLFramebufferObject *m_antialiasFbo;
    QOpenGLFramebufferObject *m_renderFbo;
    QOpenGLFramebufferObject *m_displayFbo;

    QOffscreenSurface *m_offscreenSurface;
};

QT_CANVAS3D_END_NAMESPACE

#endif // CANVAS3D_P_H

