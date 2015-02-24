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

#include "canvas3d_p.h"
#include "context3d_p.h"
#include "canvas3dcommon_p.h"
#include "canvasrendernode_p.h"
#include "teximage3d_p.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QOffscreenSurface>
#include <QtGui/QOpenGLContext>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlContext>
#include <QtCore/QElapsedTimer>

QT_BEGIN_NAMESPACE
QT_CANVAS3D_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(canvas3dinfo, "qt.canvas3d.info")
Q_LOGGING_CATEGORY(canvas3drendering, "qt.canvas3d.rendering")
Q_LOGGING_CATEGORY(canvas3dglerrors, "qt.canvas3d.glerrors")

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
 * \sa Context3D, {QtCanvas3D QML Types}
 */

/*!
 * \internal
 */
Canvas::Canvas(QQuickItem *parent):
    QQuickItem(parent),
    m_renderNodeReady(false),
    m_mainThread(QThread::currentThread()),
    m_contextThread(0),
    m_context3D(0),
    m_isFirstRender(true),
    m_fboSize(0, 0),
    m_initializedSize(1, 1),
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
    shutDown();
}

/*!
 * \internal
 */
void Canvas::shutDown()
{
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

    qCDebug(canvas3drendering) << m_contextThread << m_mainThread;

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
    qCDebug(canvas3drendering) << "Canvas3D::" << __FUNCTION__;
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
QJSValue Canvas::getContext(const QString &type)
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
QJSValue Canvas::getContext(const QString &type, const QVariantMap &options)
{
    Q_UNUSED(type);

    qCDebug(canvas3drendering) << "Canvas3D::" << __FUNCTION__
                               << "("
                               << type << ", " << options
                               << ")";

    if (!m_isContextAttribsSet) {
        // Accept passed attributes only from first call and ignore for subsequent calls
        m_isContextAttribsSet = true;
        m_contextAttribs.setFrom(options);
        qCDebug(canvas3drendering)  << "Canvas3D::" << __FUNCTION__
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

        // Initialize the swap buffer chain
        if (m_contextAttribs.depth() && m_contextAttribs.stencil() && !m_contextAttribs.antialias())
            m_fboFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        else if (m_contextAttribs.depth() && !m_contextAttribs.antialias())
            m_fboFormat.setAttachment(QOpenGLFramebufferObject::Depth);
        else if (m_contextAttribs.stencil() && !m_contextAttribs.antialias())
            m_fboFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        else
            m_fboFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);

        if (m_contextAttribs.antialias()) {
            m_antialiasFboFormat.setSamples(m_maxSamples);

            if (m_antialiasFboFormat.samples() != m_maxSamples) {
                qCWarning(canvas3drendering)  << "Failed to use " << m_maxSamples
                                              << " will use " << m_antialiasFboFormat.samples();
            }

            if (m_contextAttribs.depth() && m_contextAttribs.stencil())
                m_antialiasFboFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
            else if (m_contextAttribs.depth())
                m_antialiasFboFormat.setAttachment(QOpenGLFramebufferObject::Depth);
            else
                m_antialiasFboFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);
        }

        // Create the offscreen surface
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
            surfaceFormat.setSamples(m_antialiasFboFormat.samples());

        m_contextWindow = window();
        m_contextThread = QThread::currentThread();

        qCDebug(canvas3drendering) << "Canvas3D::" << __FUNCTION__
                                   << " Creating QOpenGLContext with surfaceFormat :"
                                   << surfaceFormat;
        m_glContext = new QOpenGLContext();
        m_glContext->setFormat(surfaceFormat);
        m_glContext->setShareContext(m_glContextQt);
        if (!m_glContext->create()) {
            qCWarning(canvas3drendering) << "Failed to create context for FBO";
            return QJSValue(QJSValue::NullValue);
        }

        m_offscreenSurface = new QOffscreenSurface();
        m_offscreenSurface->setFormat(m_glContext->format());
        m_offscreenSurface->create();

        if (!m_glContext->makeCurrent(m_offscreenSurface)) {
            qCWarning(canvas3drendering) << "Failed to make offscreen surface current";
            return QJSValue(QJSValue::NullValue);
        }

        // Initialize OpenGL functions using the created GL context
        initializeOpenGLFunctions();

        // Set the size and create FBOs
        setPixelSize(m_initializedSize);

        // Create the Context3D
        m_context3D = new CanvasContext(m_glContext, m_offscreenSurface,
                                        QQmlEngine::contextForObject(this)->engine(),
                                        m_fboSize.width(),
                                        m_fboSize.height(),
                                        m_isOpenGLES2);
        m_context3D->setCanvas(this);
        m_context3D->setDevicePixelRatio(m_devicePixelRatio);
        m_context3D->setContextAttributes(m_contextAttribs);

        emit contextChanged(m_context3D);
    }

    glFlush();
    glFinish();

    QJSValue value = QQmlEngine::contextForObject(this)->engine()->newQObject(m_context3D);
    return value;
}

/*!
 * \qmlproperty size Canvas3D::pixelSize
 * Specifies the size of the render target surface in pixels. If between logical pixels
 * (used by the Qt Quick) and actual physical on-screen pixels (used by the 3D rendering).
 */
/*!
 * \internal
 */
QSize Canvas::pixelSize()
{
    return m_fboSize;
}

/*!
 * \internal
 */
void Canvas::setPixelSize(QSize pixelSize)
{
    qCDebug(canvas3drendering) << "Canvas3D::" << __FUNCTION__
                               << "(pixelSize:" << pixelSize
                               << ")";

    if (m_fboSize == pixelSize && m_renderFbo != 0)
        return;

    m_fboSize = pixelSize;
    createFBOs();
    emit pixelSizeChanged(pixelSize);
}

/*!
 * \internal
 */
void Canvas::createFBOs()
{
    // Create the FBOs
    qCDebug(canvas3drendering) << "Canvas3D::" << __FUNCTION__
                               << "Creating front and back FBO's with attachment m_fboFormat of"
                               << m_fboFormat.attachment()
                               << "and size of" << m_fboSize;
    if (!m_contextWindow) {
        qCDebug(canvas3drendering) << "Canvas3D::" << __FUNCTION__
                                   << "No window for context, returning";
        return;
    }

    // Ensure context is current
    m_glContext->makeCurrent(m_offscreenSurface);

    // Delete existing FBOs
    if (m_displayFbo) {
        delete m_displayFbo;
        m_displayFbo = 0;
    }

    if (m_renderFbo) {
        delete m_renderFbo;
        m_renderFbo = 0;
    }

    if (m_antialiasFbo) {
        delete m_antialiasFbo;
        m_antialiasFbo = 0;
    }

    // Create FBOs
    m_displayFbo = new QOpenGLFramebufferObject(m_fboSize.width(),
                                                m_fboSize.height(),
                                                m_fboFormat);
    m_renderFbo  = new QOpenGLFramebufferObject(m_fboSize.width(),
                                                m_fboSize.height(),
                                                m_fboFormat);

    // Clear the FBOs to prevent random junk appearing on the screen
    glClearColor(0,0,0,0);
    m_displayFbo->bind();
    glViewport(0, 0,
               m_fboSize.width(),
               m_fboSize.height());
    glClear(GL_COLOR_BUFFER_BIT);
    m_renderFbo->bind();
    glViewport(0, 0,
               m_fboSize.width(),
               m_fboSize.height());
    glClear(GL_COLOR_BUFFER_BIT);

    qCDebug(canvas3drendering) << "Canvas3D::" << __FUNCTION__
                               << "Render FBO handle:" << m_renderFbo->handle()
                               << "isValid:" << m_renderFbo->isValid();

    if (m_contextAttribs.antialias()) {
        qCDebug(canvas3drendering) << "Canvas3D::" << __FUNCTION__
                                   << "Creating MSAA buffer with "
                                   << m_antialiasFboFormat.samples() << "samples"
                                   << "and attachment m_fboFormat of"
                                   << m_antialiasFboFormat.attachment();
        m_antialiasFbo = new QOpenGLFramebufferObject(
                    m_fboSize.width(),
                    m_fboSize.height(),
                    m_antialiasFboFormat);
        qCDebug(canvas3drendering) << "Canvas3D::" << __FUNCTION__
                                   << "Antialias FBO handle:" << m_antialiasFbo->handle()
                                   << "isValid:" << m_antialiasFbo->isValid();
        m_antialiasFbo->bind();
        glClear(GL_COLOR_BUFFER_BIT);
    }
}

/*!
 * \internal
 */
void Canvas::handleWindowChanged(QQuickWindow *window)
{
    qCDebug(canvas3drendering) << "Canvas3D::" << __FUNCTION__ << "(" << window << ")";
    if (!window)
        return;

    emit needRender();
}

/*!
 * \internal
 */
void Canvas::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    qCDebug(canvas3drendering) << "Canvas3D::" << __FUNCTION__;
    QQuickItem::geometryChanged(newGeometry, oldGeometry);

    m_cachedGeometry = newGeometry;

    emit needRender();
}

/*!
 * \internal
 */
void Canvas::itemChange(ItemChange change, const ItemChangeData &value)
{
    qCDebug(canvas3drendering) << "Canvas3D::" << __FUNCTION__ << change;
    QQuickItem::itemChange(change, value);

    emit needRender();
}

/*!
 * \internal
 */
CanvasContext *Canvas::context()
{
    qCDebug(canvas3drendering) << "Canvas3D::" << __FUNCTION__ << "()";
    return m_context3D;
}

/*!
 * \internal
 */
void Canvas::updateWindowParameters()
{
    qCDebug(canvas3drendering) << "Canvas3D::" << __FUNCTION__;

    // Update the device pixel ratio
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
    qCDebug(canvas3drendering) << "Canvas3D::" << __FUNCTION__;

    connect(window(), &QQuickWindow::sceneGraphInvalidated,
            this, &Canvas::shutDown);

    update();
}

/*!
 * \internal
 */
QSGNode *Canvas::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{
    qCDebug(canvas3drendering) << "Canvas3D::" << __FUNCTION__
                               << "("
                               << oldNode <<", " << data
                               << ")";
    updateWindowParameters();
    m_initializedSize = boundingRect().size().toSize();
    qCDebug(canvas3drendering) << "    size:" << m_initializedSize
                               << " devicePixelRatio:" << m_devicePixelRatio;
    if (m_runningInDesigner
            || m_initializedSize.width() <= 0
            || m_initializedSize.height() <= 0
            || !window()) {
        delete oldNode;
        qCDebug(canvas3drendering) << "    Returns null";
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
            qCDebug(canvas3drendering) << "Canvas3D::" << __FUNCTION__
                                       << " Binding current FBO to antialias FBO:"
                                       << m_antialiasFbo->handle();
            m_antialiasFbo->bind();
        } else {
            qCDebug(canvas3drendering) << "Canvas3D::" << __FUNCTION__
                                       << " Binding current FBO to render FBO:"
                                       << m_renderFbo->handle();
            m_renderFbo->bind();
        }
    } else {
        qCDebug(canvas3drendering) << "Canvas3D::" << __FUNCTION__
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
    qCDebug(canvas3drendering) << "Canvas3D::" << __FUNCTION__;

    updateWindowParameters();

    // Don't try to do anything before the render node has been created
    if (!m_renderNodeReady) {
        qCDebug(canvas3drendering) << "Canvas3D::" << __FUNCTION__
                                   << " Render node not ready, returning";
        return;
    }

    if (!m_glContext) {
        // Call the initialize function from QML/JavaScript until it calls the getContext()
        // that in turn creates the buffers.
        // Allow the JavaScript code to call the getContext() to create the context object and FBOs
        emit initGL();

        if (!m_isContextAttribsSet) {
            qCDebug(canvas3drendering) << "Canvas3D::" << __FUNCTION__
                                       << " Context attributes not set, returning";
            return;
        }

        if (!m_glContext) {
            qCDebug(canvas3drendering) << "Canvas3D::" << __FUNCTION__
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
    qCDebug(canvas3drendering) << "Canvas3D::" << __FUNCTION__
                               << " viewport set to " << viewport;

    // Check that we're complete component before drawing
    if (!isComponentComplete()) {
        qCDebug(canvas3drendering) << "Canvas3D::" << __FUNCTION__
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
        qCDebug(canvas3drendering) << "Canvas3D::" << __FUNCTION__
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
    qCDebug(canvas3drendering) << "Canvas3D::" << __FUNCTION__
                               << " Displaying texture: " << m_displayFbo->texture()
                               << " From FBO: " << m_displayFbo->handle();

    // Rebind default FBO
    QOpenGLFramebufferObject::bindDefault();

    // Notify the render node of new texture
    emit textureReady(m_displayFbo->texture(), m_fboSize, m_devicePixelRatio);
}

QT_CANVAS3D_END_NAMESPACE
QT_END_NAMESPACE
