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

#include "canvas3d_p.h"
#include "context3d_p.h"
#include "canvas3dcommon_p.h"
#include "canvasrendernode_p.h"
#include "teximage3d_p.h"

#include "typedarray/typedarray_p.h"
#include "typedarray/uint32array_p.h"
#include "typedarray/arraybuffer_p.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QOffscreenSurface>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFramebufferObject>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlContext>
#include <QtCore/QElapsedTimer>

QT_CANVAS3D_BEGIN_NAMESPACE

/*!
 * \qmltype Canvas3D
 * \since QtCanvas3D 1.0
 * \ingroup qtcanvas3d-qml-types
 * \brief Canvas that provides a 3D rendering context.
 *
 * The Canvas3D is a QML element that, when placed in your Qt Quick 2 scene, allows you to
 * get a 3D rendering context and call 3D rendering API calls through that context object.
 * Use of the rendering API requires knowledge of OpenGL like rendering APIs.
 *
 * There are two functions that are called by the Canvas3D implementation:
 * \list
 * \li initGL is emitted before the first frame is rendered, and usually during that you get
 * the 3D context and initialize resources to be used later on during the rendering cycle.
 * \li renderGL is emitted for each frame to be rendered, and usually during that you
 * submit 3D rendering calls to draw whatever 3D content you want to be displayed.
 * \endlist
 *
 * \sa Context3D, {QML Canvas 3D QML Types}
 */

/*!
 * \internal
 */
Canvas::Canvas(QQuickItem *parent):
    QQuickItem(parent),
    m_renderNodeReady(false),
    m_logAllCalls(false),
    m_logAllErrors(false),
    m_mainThread(QThread::currentThread()),
    m_contextThread(0),
    m_context3D(0),
    m_isFirstRender(true),
    m_glContext(0),
    m_glContextQt(0),
    m_contextWindow(0),
    m_fps(0),
    m_maxSamples(0),
    m_devicePixelRatio(1.0f),
    m_isOpenGLES2(false),
    m_isSoftwareRendered(false),
    m_isContextAttribsSet(false),
    m_antialiasFbo(0),
    m_renderFbo(0),
    m_displayFbo(0),
    m_offscreenSurface(0)
{
    if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__;
    connect(this, &QQuickItem::windowChanged, this, &Canvas::handleWindowChanged);
    connect(this, &Canvas::needRender, this, &Canvas::renderNext, Qt::QueuedConnection);
    setAntialiasing(false);

    // Set contents to false in case we are in qml designer to make component look nice
    m_runningInDesigner = QGuiApplication::applicationDisplayName() == "Qml2Puppet";
    setFlag(ItemHasContents, !m_runningInDesigner);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
    if (QCoreApplication::testAttribute(Qt::AA_UseSoftwareOpenGL))
        m_isSoftwareRendered = true;
#endif
}

/*!
 * \qmlsignal void Canvas::initGL()
 * Emitted once when Canvas3D is ready and OpenGL state initialization can be done by the client.
 */

/*!
 * \qmlsignal void Canvas::renderGL()
 * Emitted each time a new frame should be drawn to Canvas3D. Driven by the QML scenegraph loop.
 */

/*!
 * \internal
 */
Canvas::~Canvas()
{
    if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__;
    shutDown();
}

/*!
 * \internal
 */
void Canvas::shutDown()
{
    if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__;

    if (!m_glContext)
        return;

    disconnect(m_contextWindow, 0, this, 0);
    disconnect(this, 0, this, 0);

    m_glContext->makeCurrent(m_offscreenSurface);
    delete m_renderFbo;
    delete m_displayFbo;
    delete m_antialiasFbo;
    delete m_context3D;
    m_glContext->doneCurrent();

    if (m_logAllCalls) qDebug() << m_contextThread << m_mainThread;

    if (m_contextThread && m_contextThread != m_mainThread) {
        m_glContext->deleteLater();
        m_offscreenSurface->deleteLater();
    } else {
        delete m_glContext;
        delete m_offscreenSurface;
    }
    m_glContext = 0;
    m_glContextQt = 0;
}

/*!
 * \qmlproperty bool Canvas3D::logAllCalls
 * Specifies if all Canvas3D method calls (including internal ones) are logged to the console.
 * Defaults to \c{false}.
 */
void Canvas::setLogAllCalls(bool logCalls)
{
    if (m_logAllCalls != logCalls) {
        m_logAllCalls = logCalls;
        emit logAllCallsChanged(logCalls);
    }
}

bool Canvas::logAllCalls() const
{
    return m_logAllCalls;
}

/*!
 * \qmlproperty bool Canvas3D::logAllErrors
 * Specifies if all GL errors are checked and logged to the console. Defaults to \c{false}.
 * WARNING! Enabling this flag will have severe negative performance impact due to use of
 * \c{glGetError()} after each function call, so only enable it while debugging an issue during
 * development.
 */
void Canvas::setLogAllErrors(bool logErrors)
{
    if (m_logAllErrors != logErrors) {
        m_logAllErrors = logErrors;
        emit logAllErrorsChanged(logErrors);
    }
}

bool Canvas::logAllErrors() const
{
    return m_logAllErrors;
}

/*!
 * \qmlsignal Canvas3D::needRender()
 * This signal, if emitted, causes a re-rendering cycle to happen. Usually this is needed
 * if a value that affects the look of the 3D scene has changed, but no other mechanism
 * triggers the re-render cycle.
 */

/*!
 * \qmlsignal Canvas3D::textureReady(int id, size size, float devicePixelRatio)
 * Emitted when a new texture is ready to inform the render node.
 */

/*!
 * \qmlproperty float Canvas3D::devicePixelRatio
 * Specifies the ratio between logical pixels (used by the Qt Quick) and actual physical
 * on-screen pixels (used by the 3D rendering).
 */
float Canvas::devicePixelRatio()
{
    if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__;
    QQuickWindow *win = window();
    if (win)
        return win->devicePixelRatio();
    else
        return 1.0f;
}


/*!
 * \qmlmethod Context3D Canvas3D::getContext(string type)
 * Returns the 3D rendering context that allows 3D rendering calls to be made.
 * The \a type parameter is ignored for now, but a string is expected to be given.
 */
/*!
 * \internal
 */
CanvasContext *Canvas::getContext(const QString &type)
{
    QVariantMap map;
    return getContext(type, map);
}

/*!
 * \qmlmethod Context3D Canvas3D::getContext(string type, ContextAttributes options)
 * Returns the 3D rendering context that allows 3D rendering calls to be made.
 * The \a type parameter is ignored for now, but a string is expected to be given.
 * The \a options parameter is only parsed when the first call to getContext() is
 * made and is ignored in subsequent calls if given. If the first call is made without
 * giving the \a options parameter, then the context and render target is initialized with
 * default configuration.
 *
 * \sa ContextAttributes, Context3D
 */
/*!
 * \internal
 */
CanvasContext *Canvas::getContext(const QString &type, const QVariantMap &options)
{
    Q_UNUSED(type);

    if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__
                                << "("
                                << type << ", " << options
                                << ")";

    if (!m_isContextAttribsSet) {
        // Accept passed attributes only from first call and ignore for subsequent calls
        m_isContextAttribsSet = true;
        m_contextAttribs.setFrom(options);
        if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__
                                    << " Attribs:" << m_contextAttribs;

        // If we can't do antialiasing, ensure we don't even try to enable it
        if (m_maxSamples == 0 || m_isSoftwareRendered)
            m_contextAttribs.setAntialias(false);

        // Reflect the fact that creation of stencil attachment
        // causes the creation of depth attachment as well
        if (m_contextAttribs.stencil())
            m_contextAttribs.setDepth(true);

        // Ensure ignored attributes are left to their default state
        m_contextAttribs.setAlpha(false);
        m_contextAttribs.setPremultipliedAlpha(false);
        m_contextAttribs.setPreserveDrawingBuffer(false);
        m_contextAttribs.setPreferLowPowerToHighPerformance(false);
        m_contextAttribs.setFailIfMajorPerformanceCaveat(false);
    }

    if (!m_context3D) {
        // Create the context using current context attributes
        updateWindowParameters();

        QOpenGLFramebufferObjectFormat format;
        QOpenGLFramebufferObjectFormat antialiasFboFormat;

        // Initialize the swap buffer chain
        if (m_contextAttribs.depth() && m_contextAttribs.stencil() && !m_contextAttribs.antialias())
            format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        else if (m_contextAttribs.depth() && !m_contextAttribs.antialias())
            format.setAttachment(QOpenGLFramebufferObject::Depth);
        else if (m_contextAttribs.stencil() && !m_contextAttribs.antialias())
            format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        else
            format.setAttachment(QOpenGLFramebufferObject::NoAttachment);

        if (m_contextAttribs.antialias()) {
            antialiasFboFormat.setSamples(m_maxSamples);

            if (antialiasFboFormat.samples() != m_maxSamples) {
                qDebug() << "Failed to use " << m_maxSamples << " will use " << antialiasFboFormat.samples();
            }

            if (m_contextAttribs.depth() && m_contextAttribs.stencil())
                antialiasFboFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
            else if (m_contextAttribs.depth())
                antialiasFboFormat.setAttachment(QOpenGLFramebufferObject::Depth);
            else
                antialiasFboFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);
        }

        QSurfaceFormat surfaceFormat = m_glContextQt->format();
        if (!m_isOpenGLES2) {
            surfaceFormat.setSwapBehavior(QSurfaceFormat::SingleBuffer);
            surfaceFormat.setSwapInterval(0);
        }

        if (m_contextAttribs.alpha())
            surfaceFormat.setAlphaBufferSize(8);
        else
            surfaceFormat.setAlphaBufferSize(0);

        if (m_contextAttribs.depth())
            surfaceFormat.setDepthBufferSize(24);

        if (m_contextAttribs.stencil())
            surfaceFormat.setStencilBufferSize(8);
        else
            surfaceFormat.setStencilBufferSize(-1);

        if (m_contextAttribs.antialias())
            surfaceFormat.setSamples(antialiasFboFormat.samples());

        if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__
                                    << " Creating QOpenGLContext with surfaceFormat :"
                                    << surfaceFormat;

        m_contextWindow = window();
        m_contextThread = QThread::currentThread();
        m_glContext = new QOpenGLContext();
        m_glContext->setFormat(surfaceFormat);
        m_glContext->setShareContext(m_glContextQt);
        if (!m_glContext->create()) {
            qWarning("Failed to create context for FBO");
            return Q_NULLPTR;
        }

        m_offscreenSurface = new QOffscreenSurface();
        m_offscreenSurface->setFormat(m_glContext->format());
        m_offscreenSurface->create();

        if (!m_glContext->makeCurrent(m_offscreenSurface)) {
            qWarning("Failed to make offscreen surface current");
            return Q_NULLPTR;
        }

        // Initialize OpenGL functions using the created GL context
        initializeOpenGLFunctions();

        // Create the FBOs
        if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__
                                    << " Creating front and back FBO's with attachment format of :"
                                    << format.attachment();
        m_displayFbo = new QOpenGLFramebufferObject(m_initialisedSize, format);
        m_renderFbo  = new QOpenGLFramebufferObject(m_initialisedSize,  format);

        // Clear the FBOs to prevent random junk appearing on the screen
        glClearColor(0,0,0,0);
        m_displayFbo->bind();
        glClear(GL_COLOR_BUFFER_BIT);
        m_renderFbo->bind();
        glClear(GL_COLOR_BUFFER_BIT);

        if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__
                                    << " Render FBO handle:" << m_renderFbo->handle()
                                    << " isValid:" << m_renderFbo->isValid();

        if (m_contextAttribs.antialias()) {
            if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__
                                        << " Creating MSAA buffer with "
                                        << antialiasFboFormat.samples() << "samples "
                                        << "and attachment format of :"
                                        << antialiasFboFormat.attachment();
            m_antialiasFbo = new QOpenGLFramebufferObject(m_initialisedSize, antialiasFboFormat);
            if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__
                                        << " Antialias FBO handle:" << m_antialiasFbo->handle()
                                        << " isValid:" << m_antialiasFbo->isValid();
            m_antialiasFbo->bind();
            glClear(GL_COLOR_BUFFER_BIT);
        }

        m_context3D = new CanvasContext(m_glContext, m_offscreenSurface,
                                        QQmlEngine::contextForObject(this)->engine(),
                                        m_initialisedSize.width() * m_devicePixelRatio,
                                        m_initialisedSize.height() * m_devicePixelRatio,
                                        m_isOpenGLES2);
        m_context3D->setCanvas(this);
        m_context3D->setDevicePixelRatio(m_devicePixelRatio);
        m_context3D->setContextAttributes(m_contextAttribs);
        m_context3D->setLogAllCalls(this->logAllCalls());
        m_context3D->setLogAllErrors(this->logAllErrors());
        connect(this, &Canvas::logAllCallsChanged, m_context3D, &CanvasContext::setLogAllCalls);
        connect(this, &Canvas::logAllErrorsChanged, m_context3D, &CanvasContext::setLogAllErrors);

        emit contextChanged(m_context3D);
    }

    glFlush();
    glFinish();

    return m_context3D;
}

/*!
 * \internal
 */
void Canvas::handleWindowChanged(QQuickWindow *window)
{
    if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__ << "(" << window << ")";
    if (!window)
        return;

    emit needRender();
}

/*!
 * \internal
 */
void Canvas::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__;
    QQuickItem::geometryChanged(newGeometry, oldGeometry);

    m_cachedGeometry = newGeometry;

    emit needRender();
}

/*!
 * \internal
 */
void Canvas::itemChange(ItemChange change, const ItemChangeData &value)
{
    if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__ << change;
    QQuickItem::itemChange(change, value);

    emit needRender();
}

/*!
 * \internal
 */
CanvasContext *Canvas::context()
{
    if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__ << "()";
    return m_context3D;
}

/*!
 * \internal
 */
void Canvas::updateWindowParameters()
{
    if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__;

    // Update the device pixel ratio, window size and bounding box
    QQuickWindow *win = window();

    if (win) {
        qreal pixelRatio = win->devicePixelRatio();
        if (pixelRatio != m_devicePixelRatio) {
            m_devicePixelRatio = pixelRatio;
            emit devicePixelRatioChanged(pixelRatio);
            win->update();
        }
    }

    if (m_context3D) {
        if (m_context3D->devicePixelRatio() != m_devicePixelRatio)
            m_context3D->setDevicePixelRatio(m_devicePixelRatio);
    }
}

/*!
 * \internal
 */
void Canvas::ready()
{
    if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__;

    connect(window(), &QQuickWindow::sceneGraphInvalidated,
            this, &Canvas::shutDown);

    update();
}

/*!
 * \internal
 */
QSGNode *Canvas::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{
    if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__
                                << "("
                                << oldNode <<", " << data
                                << ")";
    updateWindowParameters();
    m_initialisedSize = boundingRect().size().toSize()  * m_devicePixelRatio;
    if (m_logAllCalls) qDebug() << "    m_initialisedSize:" << m_initialisedSize
                                << " devicePixelRatio:" << m_devicePixelRatio;
    if (m_runningInDesigner
            || m_initialisedSize.width() <= 0
            || m_initialisedSize.height() <= 0
            || !window()) {
        delete oldNode;
        if (m_logAllCalls) qDebug() << "    Returns null";
        return 0;
    }

    CanvasRenderNode *node = static_cast<CanvasRenderNode *>(oldNode);

    if (!m_glContextQt) {
        m_glContextQt = window()->openglContext();
        m_isOpenGLES2 = m_glContextQt->isOpenGLES();
        if (!m_isOpenGLES2 || m_glContextQt->format().majorVersion() >= 3)
            m_maxSamples = 4;
        ready();
        return 0;
    }

    if (!node) {
        node = new CanvasRenderNode(this, window());

        /* Set up connections to get the production of FBO textures in sync with vsync on the
         * main thread.
         *
         * When a new texture is ready on the rendering thread, we use a direct connection to
         * the texture node to let it know a new texture can be used. The node will then
         * emit pendingNewTexture which we bind to QQuickWindow::update to schedule a redraw.
         *
         * When the scene graph starts rendering the next frame, the prepareNode() function
         * is used to update the node with the new texture. Once it completes, it emits
         * textureInUse() which we connect to the FBO rendering thread's renderNext() to have
         * it start producing content into its current "back buffer".
         *
         * This FBO rendering pipeline is throttled by vsync on the scene graph rendering thread.
         */
        connect(this, &Canvas::textureReady,
                node, &CanvasRenderNode::newTexture,
                Qt::DirectConnection);

        connect(node, &CanvasRenderNode::pendingNewTexture,
                window(), &QQuickWindow::update,
                Qt::QueuedConnection);

        connect(window(), &QQuickWindow::beforeRendering,
                node, &CanvasRenderNode::prepareNode,
                Qt::DirectConnection);

        connect(node, &CanvasRenderNode::textureInUse,
                this, &Canvas::renderNext,
                Qt::QueuedConnection);

        // Get the production of FBO textures started..
        emit needRender();

        update();
    }

    node->setRect(boundingRect());
    emit needRender();

    m_renderNodeReady = true;

    return node;
}


/*!
 * \internal
 */
void Canvas::bindCurrentRenderTarget()
{
    if (m_context3D->currentFramebuffer() == 0) {
        // Bind default framebuffer
        if (m_antialiasFbo) {
            if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__
                                        << " Binding current FBO to antialias FBO:"
                                        << m_antialiasFbo->handle();
            m_antialiasFbo->bind();
        } else {
            if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__
                                        << " Binding current FBO to render FBO:"
                                        << m_renderFbo->handle();
            m_renderFbo->bind();
        }
    } else {
        if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__
                                    << " Binding current FBO to current context FBO:"
                                    << m_context3D->currentFramebuffer();
        glBindFramebuffer(GL_FRAMEBUFFER, m_context3D->currentFramebuffer());
    }
}

/*!
 * \qmlproperty uint Canvas3D::fps
 * This property specifies the current frames per seconds, the value is calculated every
 * 500 ms.
 */
uint Canvas::fps()
{
    return m_fps;
}

/*!
 * \internal
 */
void Canvas::renderNext()
{
    if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__;

    updateWindowParameters();

    // Don't try to do anything before the render node has been created
    if (!m_renderNodeReady) {
        if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__
                                    << " Render node not ready, returning";
        return;
    }

    if (!m_glContext) {
        // Call the initialize function from QML/JavaScript until it calls the getContext()
        // that in turn creates the buffers.
        // Allow the JavaScript code to call the getContext() to create the context object and FBOs
        emit initGL();

        if (!m_isContextAttribsSet) {
            if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__
                                        << " Context attributes not set, returning";
            return;
        }

        if (!m_glContext) {
            if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__
                                        << " QOpenGLContext not created, returning";
            return;
        }
    }

    // We have a GL context, make it current
    m_glContext->makeCurrent(m_offscreenSurface);

    // Bind the correct render target FBO
    bindCurrentRenderTarget();

    // Ensure we have correct clip rect set in the context
    QRect viewport = m_context3D->glViewportRect();
    glViewport(viewport.x(), viewport.y(), viewport.width(), viewport.height());
    if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__
                                << " viewport set to " << viewport;

    // Check that we're complete component before drawing
    if (!isComponentComplete()) {
        if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__
                                    << " Component is not complete, skipping drawing";
        return;
    }

    // Check if any images are loaded and need to be notified while the correct
    // GL context is current.
    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
    CanvasTextureImageFactory::factory(engine)->notifyLoadedImages();

    // Call render in QML JavaScript side
    emit renderGL();

    // Resolve MSAA
    if (m_contextAttribs.antialias()) {
        if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__
                                    << " Resolving MSAA from FBO:" << m_antialiasFbo->handle()
                                     << " to FBO:" << m_renderFbo->handle();
        QOpenGLFramebufferObject::blitFramebuffer(m_renderFbo, m_antialiasFbo);
    }

    // We need to flush the contents to the FBO before posting
    // the texture to the other thread, otherwise, we might
    // get unexpected results.
    glFlush();
    glFinish();

    // Update frames per second reading after glFinish()
    static QElapsedTimer timer;
    static int frames;

    if (frames == 0) {
        timer.start();
    } else if (timer.elapsed() > 500) {
        qreal avgtime = timer.elapsed() / (qreal) frames;
        uint avgFps = uint(1000.0 / avgtime);
        if (avgFps != m_fps) {
            m_fps = avgFps;
            emit fpsChanged(avgFps);
        }

        timer.start();
        frames = 0;
    }
    ++frames;

    // Swap
    qSwap(m_renderFbo, m_displayFbo);
    if (m_logAllCalls) qDebug() << "Canvas3D::" << __FUNCTION__
                                << " Displaying texture: " << m_displayFbo->texture()
                                << " From FBO: " << m_displayFbo->handle();

    // Rebind default FBO
    QOpenGLFramebufferObject::bindDefault();

    // Notify the render node of new texture
    emit textureReady(m_displayFbo->texture(), m_initialisedSize, m_devicePixelRatio);
}

QT_CANVAS3D_END_NAMESPACE
