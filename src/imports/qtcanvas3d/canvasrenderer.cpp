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

#include "canvasrenderer_p.h"
#include "teximage3d_p.h"
#include "renderjob_p.h"
#include "canvas3d_p.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QOffscreenSurface>
#include <QtGui/QOpenGLContext>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlContext>
#include <QtQuick/QQuickWindow>

QT_BEGIN_NAMESPACE
QT_CANVAS3D_BEGIN_NAMESPACE

const int commandQueueSize = 10000;

/*!
 * \internal
 */
CanvasRenderer::CanvasRenderer(QObject *parent):
    QObject(parent),
    m_fboSize(0, 0),
    m_initializedSize(-1, -1),
    m_glContext(0),
    m_glContextQt(0),
    m_glContextShare(0),
    m_contextWindow(0),
    m_fps(0),
    m_maxSamples(0),
    m_isOpenGLES2(false),
    m_antialias(false),
    m_preserveDrawingBuffer(false),
    m_antialiasFbo(0),
    m_renderFbo(0),
    m_displayFbo(0),
    m_recreateFbos(false),
    m_offscreenSurface(0),
    m_commandQueue(0), // command queue will be reset when context is created.
    m_executeQueue(0),
    m_executeQueueCount(0),
    m_currentFramebufferId(0),
    m_glError(0),
    m_fpsFrames(0)
{
    m_fpsTimer.start();
}

/*!
 * \internal
 *
 * Creates the shared context for Canvas. On some platforms shared context must be created
 * when the 'parent' context is not active, so we set no context as current temporarily.
 * Called from the render thread.
 */
void CanvasRenderer::createContextShare(QQuickWindow *window, const QSize &initializedSize)
{
    m_initializedSize = initializedSize;
    m_glContextQt = window->openglContext();
    m_isOpenGLES2 = m_glContextQt->isOpenGLES();
    if (!m_isOpenGLES2 || m_glContextQt->format().majorVersion() >= 3)
        m_maxSamples = 4;
    m_glContextShare = new QOpenGLContext;
    m_glContextShare->setFormat(m_glContextQt->format());
    m_glContextShare->setShareContext(m_glContextQt);
    QSurface *surface = m_glContextQt->surface();
    m_glContextQt->doneCurrent();
    if (!m_glContextShare->create()) {
        qCWarning(canvas3drendering).nospace() << "CanvasRenderer::" << __FUNCTION__
                                               << "Failed to create share context";
    }
    if (!m_glContextQt->makeCurrent(surface)) {
        qCWarning(canvas3drendering).nospace() << "CanvasRenderer::" << __FUNCTION__
                                               << " Failed to make old surface current";
    }
}

CanvasRenderer::~CanvasRenderer()
{
    shutDown();
}

/*!
 * \internal
 *
 * Cleans up the OpenGL resources.
 * Called from the render thread.
 */
void CanvasRenderer::shutDown()
{
    if (!m_glContext)
        return;

    disconnect(m_contextWindow, 0, this, 0);

    m_fps = 0;

    if ((m_commandQueue.resourceMap().size()
         || m_commandQueue.shaderMap().size()
         || m_commandQueue.programMap().size())) {
        m_commandQueue.resourceMutex()->lock();

        if (m_glContext->makeCurrent(m_offscreenSurface)) {
            QOpenGLFunctions *funcs = m_glContext->functions();

            QMap<GLint, CanvasGlCommandQueue::GlResource> resourceMap =
                    m_commandQueue.resourceMap();
            QMap<GLint, CanvasGlCommandQueue::GlResource>::const_iterator i =
                    resourceMap.constBegin();

            while (i != resourceMap.constEnd()) {
                const GLuint glId = i.value().glId;
                const CanvasGlCommandQueue::GlCommandId commandId = i.value().commandId;
                switch (commandId) {
                case CanvasGlCommandQueue::internalNoCommand: {
                    // Not allocated yet, no need to do anything
                    break;
                }
                case CanvasGlCommandQueue::glGenBuffers: {
                    funcs->glDeleteBuffers(1, &glId);
                    break;
                }
                case CanvasGlCommandQueue::glGenFramebuffers: {
                    funcs->glDeleteFramebuffers(1, &glId);
                    break;
                }
                case CanvasGlCommandQueue::glGenRenderbuffers: {
                    funcs->glDeleteRenderbuffers(1, &glId);
                    break;
                }
                case CanvasGlCommandQueue::glGenTextures: {
                    funcs->glDeleteTextures(1, &glId);
                    break;
                }
                case CanvasGlCommandQueue::glGetUniformLocation: {
                    // Nothing to do, uniforms do not actually consume resources
                    break;
                }
                default:
                    qWarning() << __FUNCTION__ << "Invalid command, cannot cleanup:"
                               << commandId << "Resource:" << glId;
                    break;
                }

                i++;
            }

            QMap<GLint, QOpenGLShader *> shaderMap = m_commandQueue.shaderMap();
            QMap<GLint, QOpenGLShader *>::const_iterator si =
                    shaderMap.constBegin();
            while (si != shaderMap.constEnd()) {
                QOpenGLShader *shader = si.value();
                delete shader;
                si++;
            }

            QMap<GLint, QOpenGLShaderProgram *> programMap = m_commandQueue.programMap();
            QMap<GLint, QOpenGLShaderProgram *>::const_iterator pi =
                    programMap.constBegin();
            while (pi != programMap.constEnd()) {
                QOpenGLShaderProgram *program = pi.value();
                delete program;
                pi++;
            }
        }
        m_commandQueue.resourceMutex()->unlock();
    }

    for (int i = 0; i < m_executeQueueCount; i++)
        m_executeQueue[i].deleteData();
    m_executeQueue.clear();

    delete m_renderFbo;
    delete m_displayFbo;
    delete m_antialiasFbo;

    m_glContext->doneCurrent();

    m_renderFbo = 0;
    m_displayFbo = 0;
    m_antialiasFbo = 0;

    delete m_glContext;
    delete m_glContextShare;

    // m_offscreenSurface is owned by main thread, as on some platforms that is required.
    m_offscreenSurface->deleteLater();
    m_offscreenSurface = 0;

    m_glContext = 0;
    m_glContextQt = 0;
    m_glContextShare = 0;

    m_currentFramebufferId = 0;
    m_forceViewportRect = QRect();
}

/*!
 *  \internal
 *
 *  Create the context and offscreen surface using current context attributes.
 *  Called from the GUI thread.
*/
bool CanvasRenderer::createContext(QQuickWindow *window,
                                   const CanvasContextAttributes &contexAttributes,
                                   GLint &maxVertexAttribs, QSize &maxSize,
                                   int &contextVersion, QSet<QByteArray> &extensions)
{
    m_antialias = contexAttributes.antialias();
    m_preserveDrawingBuffer = contexAttributes.preserveDrawingBuffer();

    m_currentFramebufferId = 0;
    m_forceViewportRect = QRect();

    // Initialize the swap buffer chain
    if (contexAttributes.depth() && contexAttributes.stencil() && !contexAttributes.antialias())
        m_fboFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    else if (contexAttributes.depth() && !contexAttributes.antialias())
        m_fboFormat.setAttachment(QOpenGLFramebufferObject::Depth);
    else if (contexAttributes.stencil() && !contexAttributes.antialias())
        m_fboFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    else
        m_fboFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);

    if (contexAttributes.antialias()) {
        m_antialiasFboFormat.setSamples(m_maxSamples);

        if (m_antialiasFboFormat.samples() != m_maxSamples) {
            qCWarning(canvas3drendering).nospace() << "CanvasRenderer::" << __FUNCTION__
                                                   << " Failed to use " << m_maxSamples
                                                   << " will use "
                                                   << m_antialiasFboFormat.samples();
        }

        if (contexAttributes.depth() && contexAttributes.stencil())
            m_antialiasFboFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        else if (contexAttributes.depth())
            m_antialiasFboFormat.setAttachment(QOpenGLFramebufferObject::Depth);
        else
            m_antialiasFboFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    }

    // Create the offscreen surface
    QSurfaceFormat surfaceFormat = m_glContextShare->format();
    if (!m_isOpenGLES2) {
        surfaceFormat.setSwapBehavior(QSurfaceFormat::SingleBuffer);
        surfaceFormat.setSwapInterval(0);
    }

    if (contexAttributes.alpha())
        surfaceFormat.setAlphaBufferSize(8);
    else
        surfaceFormat.setAlphaBufferSize(0);

    if (contexAttributes.depth())
        surfaceFormat.setDepthBufferSize(24);
    else
        surfaceFormat.setDepthBufferSize(0);

    if (contexAttributes.stencil())
        surfaceFormat.setStencilBufferSize(8);
    else
        surfaceFormat.setStencilBufferSize(0);

    if (contexAttributes.antialias())
        surfaceFormat.setSamples(m_antialiasFboFormat.samples());

    m_contextWindow = window;
    QThread *contextThread = m_glContextShare->thread();

    qCDebug(canvas3drendering).nospace() << "CanvasRenderer::" << __FUNCTION__
                                         << " Creating QOpenGLContext with surfaceFormat :"
                                         << surfaceFormat;
    m_glContext = new QOpenGLContext();
    m_glContext->setFormat(surfaceFormat);

    // Share with m_glContextShare which in turn shares with m_glContextQt.
    // In case of threaded rendering both of these live on the render
    // thread of the scenegraph. m_glContextQt may be current on that
    // thread at this point, which would fail the context creation with
    // some drivers. Hence the need for m_glContextShare.
    m_glContext->setShareContext(m_glContextShare);
    if (!m_glContext->create()) {
        qCWarning(canvas3drendering).nospace() << "CanvasRenderer::" << __FUNCTION__
                                               << " Failed to create OpenGL context for FBO";
        return false;
    }

    m_offscreenSurface = new QOffscreenSurface();
    m_offscreenSurface->setFormat(m_glContext->format());
    m_offscreenSurface->create();

    if (!m_glContext->makeCurrent(m_offscreenSurface)) {
        qCWarning(canvas3drendering).nospace() << "CanvasRenderer::" << __FUNCTION__
                                               << " Failed to make offscreen surface current";
        return false;
    }

    // Initialize OpenGL functions using the created GL context
    initializeOpenGLFunctions();

    // Get the maximum drawable size
    GLint viewportDims[2];
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, viewportDims);
    maxSize.setWidth(viewportDims[0]);
    maxSize.setHeight(viewportDims[1]);

    // Set the size and create FBOs
    setFboSize(m_initializedSize);
    m_forceViewportRect = QRect(0, 0, m_fboSize.width(), m_fboSize.height());

    m_commandQueue.resetQueue(commandQueueSize);
    m_executeQueue.resize(commandQueueSize);
    m_executeQueueCount = 0;

#ifndef QT_NO_DEBUG
    const GLubyte *version = m_glContext->functions()->glGetString(GL_VERSION);
    qCDebug(canvas3dinfo).nospace() << "CanvasRenderer::" << __FUNCTION__
                                    << "OpenGL version:" << (const char *)version;

    version = m_glContext->functions()->glGetString(GL_SHADING_LANGUAGE_VERSION);
    qCDebug(canvas3dinfo).nospace() << "CanvasRenderer::" << __FUNCTION__
                                    << "GLSL version:" << (const char *)version;

    qCDebug(canvas3dinfo).nospace() << "CanvasRenderer::" << __FUNCTION__
                                    << "EXTENSIONS: " << extensions;
#endif

    m_glContext->functions()->glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);

    contextVersion = m_glContext->format().majorVersion();

    extensions = m_glContext->extensions();

    logGlErrors(__FUNCTION__);

    if (m_glContext->thread() != contextThread) {
        m_glContext->doneCurrent();
        m_glContext->moveToThread(contextThread);
    }


    return true;
}

/*!
 * \internal
 *
 * Renders the command queue in the correct context.
 * Called from the render thread.
 */
void CanvasRenderer::render()
{
    // Skip render if there is no context or nothing to render
    if (m_glContext && m_executeQueueCount) {
        // Render to offscreen fbo
        QOpenGLContext *oldContext = QOpenGLContext::currentContext();
        QSurface *oldSurface = oldContext->surface();

        makeCanvasContextCurrent();
        executeCommandQueue();

        // Restore Qt context
        if (!oldContext->makeCurrent(oldSurface)) {
            qCWarning(canvas3drendering).nospace() << "Canvas3D::" << __FUNCTION__
                                                   << " Failed to make old surface current";
        }
    }
}

/*!
 * \internal
 *
 * Sets framebuffer size.
 * Called from the GUI thread.
 */
void CanvasRenderer::setFboSize(const QSize &fboSize)
{
    qCDebug(canvas3drendering).nospace() << "CanvasRenderer::" << __FUNCTION__
                                         << "(setFboSize:" << fboSize
                                         << ")";

    if (m_fboSize == fboSize && m_renderFbo != 0)
        return;

    m_fboSize = fboSize;
    m_recreateFbos = true;
}

/*!
 * \internal
 *
 * Creates framebuffers.
 * This method is called from the render thread and in the correct context.
 */
void CanvasRenderer::createFBOs()
{
    qCDebug(canvas3drendering).nospace() << "CanvasRenderer::" << __FUNCTION__ << "()";

    if (!m_glContext) {
        qCDebug(canvas3drendering).nospace() << "CanvasRenderer::" << __FUNCTION__
                                             << " No OpenGL context created, returning";
        return;
    }

    if (!m_offscreenSurface) {
        qCDebug(canvas3drendering).nospace() << "CanvasRenderer::" << __FUNCTION__
                                             << " No offscreen surface created, returning";
        return;
    }

    // Ensure context is current
    if (!m_glContext->makeCurrent(m_offscreenSurface)) {
        qCWarning(canvas3drendering).nospace() << "CanvasRenderer::" << __FUNCTION__
                                               << " Failed to make offscreen surface current";
        return;
    }

    // Store current clear color and the bound texture
    GLint texBinding2D;
    GLfloat clearColor[4];
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &texBinding2D);
    glGetFloatv(GL_COLOR_CLEAR_VALUE, clearColor);

    // Store existing FBOs, don't delete before we've created the new ones
    // so that we get fresh texture and FBO id's for the newly created objects.
    QOpenGLFramebufferObject *displayFBO = m_displayFbo;
    QOpenGLFramebufferObject *renderFbo = m_renderFbo;
    QOpenGLFramebufferObject *antialiasFbo = m_antialiasFbo;

    // Create FBOs
    qCDebug(canvas3drendering).nospace() << "CanvasRenderer::" << __FUNCTION__
                                         << " Creating front and back FBO's with"
                                         << " attachment format:" << m_fboFormat.attachment()
                                         << " and size:" << m_fboSize;
    m_displayFbo = new QOpenGLFramebufferObject(m_fboSize.width(),
                                                m_fboSize.height(),
                                                m_fboFormat);
    m_renderFbo  = new QOpenGLFramebufferObject(m_fboSize.width(),
                                                m_fboSize.height(),
                                                m_fboFormat);

    // Clear the FBOs to prevent random junk appearing on the screen
    // Note: Viewport may not be changed automatically
    glClearColor(0,0,0,0);
    m_displayFbo->bind();
    glClear(GL_COLOR_BUFFER_BIT);
    m_renderFbo->bind();
    glClear(GL_COLOR_BUFFER_BIT);

    qCDebug(canvas3drendering).nospace() << "CanvasRenderer::" << __FUNCTION__
                                         << " Render FBO handle:" << m_renderFbo->handle()
                                         << " isValid:" << m_renderFbo->isValid();

    if (m_antialias) {
        qCDebug(canvas3drendering).nospace() << "CanvasRenderer::" << __FUNCTION__
                                             << "Creating MSAA buffer with "
                                             << m_antialiasFboFormat.samples() << " samples "
                                             << " and attachment format of "
                                             << m_antialiasFboFormat.attachment();
        m_antialiasFbo = new QOpenGLFramebufferObject(
                    m_fboSize.width(),
                    m_fboSize.height(),
                    m_antialiasFboFormat);
        qCDebug(canvas3drendering).nospace() << "CanvasRenderer::" << __FUNCTION__
                                             << " Antialias FBO handle:" << m_antialiasFbo->handle()
                                             << " isValid:" << m_antialiasFbo->isValid();
        m_antialiasFbo->bind();
        glClear(GL_COLOR_BUFFER_BIT);
    }

    // FBO ids and texture id's have been generated, we can now free the existing ones.
    delete displayFBO;
    delete renderFbo;
    delete antialiasFbo;

    // Store the correct texture binding
    glBindTexture(GL_TEXTURE_2D, texBinding2D);
    glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);

    if (m_currentFramebufferId)
        bindCurrentRenderTarget();

    logGlErrors(__FUNCTION__);
}

/*!
 * \internal
 *
 * This method is called from the render thread and in the correct context.
 */
void CanvasRenderer::logGlErrors(const char *funcName)
{
    if (canvas3dglerrors().isDebugEnabled()) {
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            // Merge any GL errors with internal errors so that we don't lose them
            switch (err) {
            case GL_INVALID_ENUM:
                m_glError |= CanvasContext::CANVAS_INVALID_ENUM;
                break;
            case GL_INVALID_VALUE:
                m_glError |= CanvasContext::CANVAS_INVALID_VALUE;
                break;
            case GL_INVALID_OPERATION:
                m_glError |= CanvasContext::CANVAS_INVALID_OPERATION;
                break;
            case GL_OUT_OF_MEMORY:
                m_glError |= CanvasContext::CANVAS_OUT_OF_MEMORY;
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                m_glError |= CanvasContext::CANVAS_INVALID_FRAMEBUFFER_OPERATION;
                break;
            default:
                break;
            }
            qCWarning(canvas3dglerrors).nospace() << "CanvasRenderer::" << funcName
                                                  << ": OpenGL ERROR: "
                                                  << err;
        }
    }
}

/*!
 * \internal
 *
 * Moves commands from command queue to execution queue.
 * Called from the render thread when the GUI thread is blocked.
 */
void CanvasRenderer::transferCommands()
{
    if (m_glContext)
        m_executeQueueCount = m_commandQueue.transferCommands(m_executeQueue);
}

/*!
 * \internal
 *
 * Bind the correct target framebuffer for rendering.
 * This method is called from the render thread and in the correct context.
 */
void CanvasRenderer::bindCurrentRenderTarget()
{
    qCDebug(canvas3drendering).nospace() << "CanvasRenderer::" << __FUNCTION__ << "()";

    if (m_currentFramebufferId == 0) {
        // Bind default framebuffer
        if (m_antialiasFbo) {
            qCDebug(canvas3drendering).nospace() << "CanvasRenderer::" << __FUNCTION__
                                                 << " Binding current FBO to antialias FBO:"
                                                 << m_antialiasFbo->handle();
            m_antialiasFbo->bind();
        } else {
            qCDebug(canvas3drendering).nospace() << "CanvasRenderer::" << __FUNCTION__
                                                 << " Binding current FBO to render FBO:"
                                                 << m_renderFbo->handle();
            m_renderFbo->bind();
        }
    } else {
        qCDebug(canvas3drendering).nospace() << "CanvasRenderer::" << __FUNCTION__
                                             << " Binding current FBO to current Context3D FBO:"
                                             << m_currentFramebufferId;
        glBindFramebuffer(GL_FRAMEBUFFER, m_currentFramebufferId);
    }
    logGlErrors(__FUNCTION__);
}

/*!
 * \internal
 *
 * Makes the canvas context current, if it exists.
 * Called from the render thread.
 */
void CanvasRenderer::makeCanvasContextCurrent()
{
    if (!m_glContext)
        return;

    if (!m_glContext->makeCurrent(m_offscreenSurface)) {
        qCWarning(canvas3drendering).nospace() << "CanvasRenderer::" << __FUNCTION__
                                               << " Failed to make offscreen surface current";
    }
}

/*!
 * \internal
 *
 * Executes the command queue.
 * This method is called from the render thread and in the correct context.
 */
void CanvasRenderer::executeCommandQueue()
{
    if (!m_glContext)
        return;

    if (m_recreateFbos) {
        createFBOs();
        m_recreateFbos = false;
    }
    qCDebug(canvas3drendering).nospace() << "CanvasRenderer::" << __FUNCTION__;

    if (!m_glContext->makeCurrent(m_offscreenSurface)) {
        qCWarning(canvas3drendering).nospace() << "CanvasRenderer::" << __FUNCTION__
                                               << " Failed to make offscreen surface current";
        return;
    }

    // Bind the correct render target FBO
    bindCurrentRenderTarget();

    // Ensure we have correct viewport set in the context
    glViewport(m_forceViewportRect.x(), m_forceViewportRect.y(),
               m_forceViewportRect.width(), m_forceViewportRect.height());
    qCDebug(canvas3drendering).nospace() << "CanvasRenderer::" << __FUNCTION__
                                         << " Viewport set to " << m_forceViewportRect;

    GLuint u1(0); // A generic variable used in the following switch statement

    // Execute the execution queue
    for (int i = 0; i < m_executeQueueCount; i++) {
        GlCommand &command = m_executeQueue[i];
        switch (command.id) {
        case CanvasGlCommandQueue::glActiveTexture: {
            glActiveTexture(GLenum(command.i1));
            break;
        }
        case CanvasGlCommandQueue::glAttachShader: {
            QOpenGLShaderProgram *program = m_commandQueue.getProgram(command.i1);
            QOpenGLShader *shader = m_commandQueue.getShader(command.i2);
            bool success = false;
            if (program && shader)
                success = program->addShader(shader);
            if (!success) {
                m_glError |= CanvasContext::CANVAS_INVALID_VALUE;
                qCWarning(canvas3dglerrors).nospace() << "CanvasRenderer::" << __FUNCTION__
                                                      << ": Failed to attach shader "
                                                      << shader << " to program " << program;
            }
            break;
        }
        case CanvasGlCommandQueue::glBindAttribLocation: {
            QOpenGLShaderProgram *program = m_commandQueue.getProgram(command.i1);
            if (program) {
                program->bindAttributeLocation(*command.data, command.i2);
            } else {
                m_glError |= CanvasContext::CANVAS_INVALID_VALUE;
                qCWarning(canvas3dglerrors).nospace() << "CanvasRenderer::" << __FUNCTION__
                                                      << ": Failed to bind attribute ("
                                                      << command.data << ") to program " << program;
            }
            command.deleteData();
            break;
        }
        case CanvasGlCommandQueue::glBindBuffer: {
            glBindBuffer(GLenum(command.i1), m_commandQueue.getGlId(command.i2));
            break;
        }
        case CanvasGlCommandQueue::glBindFramebuffer: {
            m_currentFramebufferId = m_commandQueue.getGlId(command.i1);
            // Special logic for frame buffer binding
            bindCurrentRenderTarget();
            break;
        }
        case CanvasGlCommandQueue::glBindRenderbuffer: {
            glBindRenderbuffer(GLenum(command.i1), m_commandQueue.getGlId(command.i2));
            break;
        }
        case CanvasGlCommandQueue::glBindTexture: {
            glBindTexture(GLenum(command.i1), m_commandQueue.getGlId(command.i2));
            break;
        }
        case CanvasGlCommandQueue::glBlendColor: {
            glBlendColor(GLclampf(command.f1), GLclampf(command.f2),
                         GLclampf(command.f3), GLclampf(command.f4));
            break;
        }
        case CanvasGlCommandQueue::glBlendEquation: {
            glBlendEquation(GLenum(command.i1));
            break;
        }
        case CanvasGlCommandQueue::glBlendEquationSeparate: {
            glBlendEquationSeparate(GLenum(command.i1), GLenum(command.i2));
            break;
        }
        case CanvasGlCommandQueue::glBlendFunc: {
            glBlendFunc(GLenum(command.i1), GLenum(command.i2));
            break;
        }
        case CanvasGlCommandQueue::glBlendFuncSeparate: {
            glBlendFuncSeparate(GLenum(command.i1), GLenum(command.i2),
                                GLenum(command.i3), GLenum(command.i4));
            break;
        }
        case CanvasGlCommandQueue::glBufferData: {
            const void *data = 0;
            if (command.data)
                data = static_cast<const void *>(command.data->constData());

            glBufferData(GLenum(command.i1), qopengl_GLsizeiptr(command.i2),
                         data, GLenum(command.i3));

            command.deleteData();
            break;
        }
        case CanvasGlCommandQueue::glBufferSubData: {
            const void *data = static_cast<const void *>(command.data->constData());
            glBufferSubData(GLenum(command.i1), qopengl_GLintptr(command.i2),
                            qopengl_GLsizeiptr(command.data->size()), data);
            command.deleteData();
            break;
        }
        case CanvasGlCommandQueue::glClear: {
            glClear(GLbitfield(command.i1));
            break;
        }
        case CanvasGlCommandQueue::glClearColor: {
            glClearColor(GLclampf(command.f1), GLclampf(command.f2),
                         GLclampf(command.f3), GLclampf(command.f4));
            break;
        }
        case CanvasGlCommandQueue::glClearDepthf: {
            glClearDepthf(GLclampf(command.f1));
            break;
        }
        case CanvasGlCommandQueue::glClearStencil: {
            glClearStencil(command.i1);
            break;
        }
        case CanvasGlCommandQueue::glColorMask: {
            glColorMask(GLboolean(command.i1), GLboolean(command.i2),
                        GLboolean(command.i3), GLboolean(command.i4));
            break;
        }
        case CanvasGlCommandQueue::glCompileShader: {
            QOpenGLShader *shader = m_commandQueue.getShader(command.i1);
            bool success = false;
            if (shader)
                success = shader->compileSourceCode(*command.data);
            if (!success) {
                m_glError |= CanvasContext::CANVAS_INVALID_VALUE;
                qCWarning(canvas3dglerrors).nospace() << "CanvasRenderer::" << __FUNCTION__
                                                      << ": Failed to compile shader "
                                                      << shader;
            }
            command.deleteData();
            break;
        }
        case CanvasGlCommandQueue::glCompressedTexImage2D: {
            const void *data = static_cast<const void *>(command.data->constData());
            glCompressedTexImage2D(GLenum(command.i1), command.i2, GLenum(command.i3),
                                   GLsizei(command.i4), GLsizei(command.i5), command.i6,
                                   GLsizei(command.data->size()), data);
            command.deleteData();
            break;
        }
        case CanvasGlCommandQueue::glCompressedTexSubImage2D: {
            const void *data = static_cast<const void *>(command.data->constData());
            glCompressedTexSubImage2D(GLenum(command.i1), command.i2, command.i3, command.i4,
                                      GLsizei(command.i5), GLsizei(command.i6), GLenum(command.i7),
                                      GLsizei(command.data->size()), data);
            command.deleteData();
            break;
        }
        case CanvasGlCommandQueue::glCopyTexImage2D: {
            glCopyTexImage2D(GLenum(command.i1), command.i2, GLenum(command.i3), command.i4,
                             command.i5, GLsizei(command.i6), GLsizei(command.i7), command.i8);
            break;
        }
        case CanvasGlCommandQueue::glCopyTexSubImage2D: {
            glCopyTexSubImage2D(GLenum(command.i1), command.i2, command.i3, command.i4, command.i5,
                                command.i6, GLsizei(command.i7), GLsizei(command.i8));
            break;
        }
        case CanvasGlCommandQueue::glCreateProgram: {
            QOpenGLShaderProgram *program = new QOpenGLShaderProgram();
            m_commandQueue.setProgramToMap(command.i1, program);
            break;
        }
        case CanvasGlCommandQueue::glCreateShader: {
            QOpenGLShader::ShaderType type = (command.i1 == GL_VERTEX_SHADER)
                    ? QOpenGLShader::Vertex : QOpenGLShader::Fragment;
            QOpenGLShader *shader = new QOpenGLShader(type);
            m_commandQueue.setShaderToMap(command.i2, shader);
            break;
        }
        case CanvasGlCommandQueue::glCullFace: {
            glCullFace(GLenum(command.i1));
            break;
        }
        case CanvasGlCommandQueue::glDeleteBuffers: {
            u1 = m_commandQueue.takeSingleIdParam(command);
            glDeleteBuffers(1, &u1);
            break;
        }
        case CanvasGlCommandQueue::glDeleteFramebuffers: {
            u1 = m_commandQueue.takeSingleIdParam(command);
            glDeleteFramebuffers(1, &u1);
            break;
        }
        case CanvasGlCommandQueue::glDeleteProgram: {
            delete m_commandQueue.takeProgramFromMap(command.i1);
            break;
        }
        case CanvasGlCommandQueue::glDeleteRenderbuffers: {
            u1 = m_commandQueue.takeSingleIdParam(command);
            glDeleteRenderbuffers(1, &u1);
            break;
        }
        case CanvasGlCommandQueue::glDeleteShader: {
            delete m_commandQueue.takeShaderFromMap(command.i1);
            break;
        }
        case CanvasGlCommandQueue::glDeleteTextures: {
            u1 = m_commandQueue.takeSingleIdParam(command);
            glDeleteTextures(1, &u1);
            break;
        }
        case CanvasGlCommandQueue::glDepthFunc: {
            glDepthFunc(GLenum(command.i1));
            break;
        }
        case CanvasGlCommandQueue::glDepthMask: {
            glDepthMask(GLboolean(command.i1));
            break;
        }
        case CanvasGlCommandQueue::glDepthRangef: {
            glDepthRangef(GLclampf(command.f1), GLclampf(command.f2));
            break;
        }
        case CanvasGlCommandQueue::glDetachShader: {
            QOpenGLShaderProgram *program = m_commandQueue.getProgram(command.i1);
            QOpenGLShader *shader = m_commandQueue.getShader(command.i2);
            if (program && shader) {
                program->removeShader(shader);
            } else {
                m_glError |= CanvasContext::CANVAS_INVALID_VALUE;
                qCWarning(canvas3dglerrors).nospace() << "CanvasRenderer::" << __FUNCTION__
                                                      << ": Failed to detach shader "
                                                      << shader << " from program " << program;
            }
            break;
        }
        case CanvasGlCommandQueue::glDisable: {
            glDisable(GLenum(command.i1));
            break;
        }
        case CanvasGlCommandQueue::glDisableVertexAttribArray: {
            glDisableVertexAttribArray(GLuint(command.i1));
            break;
        }
        case CanvasGlCommandQueue::glDrawArrays: {
            glDrawArrays(GLenum(command.i1), command.i2, GLsizei(command.i3));
            break;
        }
        case CanvasGlCommandQueue::glDrawElements: {
            glDrawElements(GLenum(command.i1), GLsizei(command.i2), GLenum(command.i3),
                           reinterpret_cast<GLvoid *>(command.i4));
            break;
        }
        case CanvasGlCommandQueue::glEnable: {
            glEnable(GLenum(command.i1));
            break;
        }
        case CanvasGlCommandQueue::glEnableVertexAttribArray: {
            glEnableVertexAttribArray(GLuint(command.i1));
            break;
        }
        case CanvasGlCommandQueue::glFlush: {
            glFlush();
            break;
        }
        case CanvasGlCommandQueue::glFramebufferRenderbuffer: {
            glFramebufferRenderbuffer(GLenum(command.i1), GLenum(command.i2), GLenum(command.i3),
                                      m_commandQueue.getGlId(command.i4));
            break;
        }
        case CanvasGlCommandQueue::glFramebufferTexture2D: {
            glFramebufferTexture2D(GLenum(command.i1), GLenum(command.i2), GLenum(command.i3),
                                   m_commandQueue.getGlId(command.i4), command.i5);
            break;
        }
        case CanvasGlCommandQueue::glFrontFace: {
            glFrontFace(GLenum(command.i1));
            break;
        }
        case CanvasGlCommandQueue::glGenBuffers: {
            glGenBuffers(1, &u1);
            m_commandQueue.handleGenerateCommand(command, u1);
            break;
        }
        case CanvasGlCommandQueue::glGenerateMipmap: {
            glGenerateMipmap(GLenum(command.i1));
            break;
        }
        case CanvasGlCommandQueue::glGenFramebuffers: {
            glGenFramebuffers(1, &u1);
            m_commandQueue.handleGenerateCommand(command, u1);
            break;
        }
        case CanvasGlCommandQueue::glGenRenderbuffers: {
            glGenRenderbuffers(1, &u1);
            m_commandQueue.handleGenerateCommand(command, u1);
            break;
        }
        case CanvasGlCommandQueue::glGenTextures: {
            glGenTextures(1, &u1);
            m_commandQueue.handleGenerateCommand(command, u1);
            break;
        }
        case CanvasGlCommandQueue::glGetUniformLocation: {
            QOpenGLShaderProgram *program = m_commandQueue.getProgram(command.i2);
            const GLuint glLocation = program ? program->uniformLocation(*command.data)
                                              : GLuint(-1);
            // command.i1 is the location id, so use standard handler function to generate mapping
            m_commandQueue.handleGenerateCommand(command, glLocation);
            command.deleteData();
            break;
        }
        case CanvasGlCommandQueue::glHint: {
            glHint(GLenum(command.i1), GLenum(command.i2));
            break;
        }
        case CanvasGlCommandQueue::glLineWidth: {
            glLineWidth(command.f1);
            break;
        }
        case CanvasGlCommandQueue::glLinkProgram: {
            QOpenGLShaderProgram *program = m_commandQueue.getProgram(command.i1);
            bool success = false;
            if (program)
                success = program->link();
            if (!success) {
                m_glError |= CanvasContext::CANVAS_INVALID_VALUE;
                qCWarning(canvas3dglerrors).nospace() << "CanvasRenderer::" << __FUNCTION__
                                                      << ": Failed to link program: "
                                                      << program;
            }
            break;
        }
        case CanvasGlCommandQueue::glPixelStorei: {
            glPixelStorei(GLenum(command.i1), command.i2);
            break;
        }
        case CanvasGlCommandQueue::glPolygonOffset: {
            glPolygonOffset(command.f1, command.f2);
            break;
        }
        case CanvasGlCommandQueue::glRenderbufferStorage: {
            glRenderbufferStorage(GLenum(command.i1), GLenum(command.i2),
                                  GLsizei(command.i3), GLsizei(command.i4));
            break;
        }
        case CanvasGlCommandQueue::glSampleCoverage: {
            glSampleCoverage(GLclampf(command.f1), GLboolean(command.i1));
            break;
        }
        case CanvasGlCommandQueue::glScissor: {
            glScissor(command.i1, command.i2, GLsizei(command.i3), GLsizei(command.i4));
            break;
        }
        case CanvasGlCommandQueue::glStencilFunc: {
            glStencilFunc(GLenum(command.i1), command.i2, GLuint(command.i3));
            break;
        }
        case CanvasGlCommandQueue::glStencilFuncSeparate: {
            glStencilFuncSeparate(GLenum(command.i1), GLenum(command.i2),
                                  command.i3, GLuint(command.i4));
            break;
        }
        case CanvasGlCommandQueue::glStencilMask: {
            glStencilMask(GLuint(command.i1));
            break;
        }
        case CanvasGlCommandQueue::glStencilMaskSeparate: {
            glStencilMaskSeparate(GLenum(command.i1), GLuint(command.i2));
            break;
        }
        case CanvasGlCommandQueue::glStencilOp: {
            glStencilOp(GLenum(command.i1), GLenum(command.i2), GLenum(command.i3));
            break;
        }
        case CanvasGlCommandQueue::glStencilOpSeparate: {
            glStencilOpSeparate(GLenum(command.i1), GLenum(command.i2),
                                GLenum(command.i3), GLenum(command.i4));
            break;
        }
        case CanvasGlCommandQueue::glTexImage2D: {
            glTexImage2D(GLenum(command.i1), command.i2, command.i3, GLsizei(command.i4),
                         GLsizei(command.i5), command.i6, GLenum(command.i7), GLenum(command.i8),
                         reinterpret_cast<const GLvoid *>(command.data->constData()));
            command.deleteData();
            break;
        }
        case CanvasGlCommandQueue::glTexParameterf: {
            glTexParameterf(GLenum(command.i1), GLenum(command.i2), command.f1);
            break;
        }
        case CanvasGlCommandQueue::glTexParameteri: {
            glTexParameteri(GLenum(command.i1), GLenum(command.i2), command.i3);
            break;
        }
        case CanvasGlCommandQueue::glTexSubImage2D: {
            glTexSubImage2D(GLenum(command.i1), command.i2, command.i3, command.i4,
                            GLsizei(command.i5), GLsizei(command.i6), GLenum(command.i7),
                            GLenum(command.i8),
                            reinterpret_cast<const GLvoid *>(command.data->constData()));
            command.deleteData();
            break;
        }
        case CanvasGlCommandQueue::glUniform1f: {
            glUniform1f(GLint(m_commandQueue.getGlId(command.i1)), command.f1);
            break;
        }
        case CanvasGlCommandQueue::glUniform2f: {
            glUniform2f(GLint(m_commandQueue.getGlId(command.i1)), command.f1, command.f2);
            break;
        }
        case CanvasGlCommandQueue::glUniform3f: {
            glUniform3f(GLint(m_commandQueue.getGlId(command.i1)), command.f1, command.f2,
                        command.f3);
            break;
        }
        case CanvasGlCommandQueue::glUniform4f: {
            glUniform4f(GLint(m_commandQueue.getGlId(command.i1)), command.f1, command.f2,
                        command.f3, command.f4);
            break;
        }
        case CanvasGlCommandQueue::glUniform1i: {
            glUniform1i(GLint(m_commandQueue.getGlId(command.i1)), command.i2);
            break;
        }
        case CanvasGlCommandQueue::glUniform2i: {
            glUniform2i(GLint(m_commandQueue.getGlId(command.i1)), command.i2, command.i3);
            break;
        }
        case CanvasGlCommandQueue::glUniform3i: {
            glUniform3i(GLint(m_commandQueue.getGlId(command.i1)), command.i2, command.i3,
                        command.i4);
            break;
        }
        case CanvasGlCommandQueue::glUniform4i: {
            glUniform4i(GLint(m_commandQueue.getGlId(command.i1)), command.i2, command.i3,
                        command.i4, command.i5);
            break;
        }
        case CanvasGlCommandQueue::glUniform1fv: {
            glUniform1fv(GLint(m_commandQueue.getGlId(command.i1)), GLsizei(command.i2),
                         reinterpret_cast<const GLfloat *>(command.data->constData()));
            command.deleteData();
            break;
        }
        case CanvasGlCommandQueue::glUniform2fv: {
            glUniform2fv(GLint(m_commandQueue.getGlId(command.i1)), GLsizei(command.i2),
                         reinterpret_cast<const GLfloat *>(command.data->constData()));
            command.deleteData();
            break;
        }
        case CanvasGlCommandQueue::glUniform3fv: {
            glUniform3fv(GLint(m_commandQueue.getGlId(command.i1)), GLsizei(command.i2),
                         reinterpret_cast<const GLfloat *>(command.data->constData()));
            command.deleteData();
            break;
        }
        case CanvasGlCommandQueue::glUniform4fv: {
            glUniform4fv(GLint(m_commandQueue.getGlId(command.i1)), GLsizei(command.i2),
                         reinterpret_cast<const GLfloat *>(command.data->constData()));
            command.deleteData();
            break;
        }
        case CanvasGlCommandQueue::glUniform1iv: {
            glUniform1iv(GLint(m_commandQueue.getGlId(command.i1)), GLsizei(command.i2),
                         reinterpret_cast<const GLint *>(command.data->constData()));
            command.deleteData();
            break;
        }
        case CanvasGlCommandQueue::glUniform2iv: {
            glUniform2iv(GLint(m_commandQueue.getGlId(command.i1)), GLsizei(command.i2),
                         reinterpret_cast<const GLint *>(command.data->constData()));
            command.deleteData();
            break;
        }
        case CanvasGlCommandQueue::glUniform3iv: {
            glUniform3iv(GLint(m_commandQueue.getGlId(command.i1)), GLsizei(command.i2),
                         reinterpret_cast<const GLint *>(command.data->constData()));
            command.deleteData();
            break;
        }
        case CanvasGlCommandQueue::glUniform4iv: {
            glUniform4iv(GLint(m_commandQueue.getGlId(command.i1)), GLsizei(command.i2),
                         reinterpret_cast<const GLint *>(command.data->constData()));
            command.deleteData();
            break;
        }
        case CanvasGlCommandQueue::glUniformMatrix2fv: {
            glUniformMatrix2fv(GLint(m_commandQueue.getGlId(command.i1)),
                               GLsizei(command.i2), GLboolean(command.i3),
                               reinterpret_cast<const GLfloat *>(command.data->constData()));
            command.deleteData();
            break;
        }
        case CanvasGlCommandQueue::glUniformMatrix3fv: {
            glUniformMatrix3fv(GLint(m_commandQueue.getGlId(command.i1)),
                               GLsizei(command.i2), GLboolean(command.i3),
                               reinterpret_cast<const GLfloat *>(command.data->constData()));
            command.deleteData();
            break;
        }
        case CanvasGlCommandQueue::glUniformMatrix4fv: {
            glUniformMatrix4fv(GLint(m_commandQueue.getGlId(command.i1)),
                               GLsizei(command.i2), GLboolean(command.i3),
                               reinterpret_cast<const GLfloat *>(command.data->constData()));
            command.deleteData();
            break;
        }
        case CanvasGlCommandQueue::glUseProgram: {
            QOpenGLShaderProgram *program = m_commandQueue.getProgram(command.i1);
            if (program) {
                glUseProgram(program->programId());
            } else {
                m_glError |= CanvasContext::CANVAS_INVALID_VALUE;
                qCWarning(canvas3dglerrors).nospace() << "CanvasRenderer::" << __FUNCTION__
                                                      << ": Failed to bind program: "
                                                      << program;
            }
            break;
        }
        case CanvasGlCommandQueue::glValidateProgram: {
            QOpenGLShaderProgram *program = m_commandQueue.getProgram(command.i1);
            if (program) {
                glValidateProgram(program->programId());
            } else {
                m_glError |= CanvasContext::CANVAS_INVALID_VALUE;
                qCWarning(canvas3dglerrors).nospace() << "CanvasRenderer::" << __FUNCTION__
                                                      << ": Failed to validate program, "
                                                      << "invalid program id";
            }
            break;
        }
        case CanvasGlCommandQueue::glVertexAttrib1f: {
            glVertexAttrib1f(GLuint(command.i1), command.f1);
            break;
        }
        case CanvasGlCommandQueue::glVertexAttrib2f: {
            glVertexAttrib2f(GLuint(command.i1), command.f1, command.f2);
            break;
        }
        case CanvasGlCommandQueue::glVertexAttrib3f: {
            glVertexAttrib3f(GLuint(command.i1), command.f1, command.f2, command.f3);
            break;
        }
        case CanvasGlCommandQueue::glVertexAttrib4f: {
            glVertexAttrib4f(GLuint(command.i1), command.f1, command.f2, command.f3, command.f4);
            break;
        }
        case CanvasGlCommandQueue::glVertexAttrib1fv: {
            glVertexAttrib1fv(GLuint(command.i1),
                              reinterpret_cast<const GLfloat *>(command.data->constData()));
            command.deleteData();
            break;
        }
        case CanvasGlCommandQueue::glVertexAttrib2fv: {
            glVertexAttrib2fv(GLuint(command.i1),
                              reinterpret_cast<const GLfloat *>(command.data->constData()));
            command.deleteData();
            break;
        }
        case CanvasGlCommandQueue::glVertexAttrib3fv: {
            glVertexAttrib3fv(GLuint(command.i1),
                              reinterpret_cast<const GLfloat *>(command.data->constData()));
            command.deleteData();
            break;
        }
        case CanvasGlCommandQueue::glVertexAttrib4fv: {
            glVertexAttrib4fv(GLuint(command.i1),
                              reinterpret_cast<const GLfloat *>(command.data->constData()));
            command.deleteData();
            break;
        }
        case CanvasGlCommandQueue::glVertexAttribPointer: {
            glVertexAttribPointer(GLuint(command.i1), command.i2, GLenum(command.i3),
                                  GLboolean(command.i4), GLsizei(command.i5),
                                  reinterpret_cast<const GLvoid *>(command.i6));
            break;
        }
        case CanvasGlCommandQueue::glViewport: {
            glViewport(command.i1, command.i2, GLsizei(command.i3), GLsizei(command.i4));
            if (command.i3 < 0 || command.i4 < 0) {
                m_forceViewportRect = QRect();
            } else {
                m_forceViewportRect.setX(command.i1);
                m_forceViewportRect.setY(command.i2);
                m_forceViewportRect.setWidth(command.i3);
                m_forceViewportRect.setHeight(command.i4);
            }
            break;
        }
        case CanvasGlCommandQueue::internalClearLocation: {
            // Used to clear a mapped uniform location from map when no longer needed
            m_commandQueue.removeResourceIdFromMap(command.i1);
            break;
        }
        case CanvasGlCommandQueue::internalTextureComplete: {
            finalizeTexture();
            bindCurrentRenderTarget();
            break;
        }
        default: {
            qWarning() << __FUNCTION__
                       << "Unsupported GL command handled:" << command.id;
            break;
        }
        }
        logGlErrors(__FUNCTION__);
    }

    m_executeQueueCount = 0;

    // Rebind default FBO
    QOpenGLFramebufferObject::bindDefault();
}

/*!
 * \internal
 *
 * Executes a single GlSyncCommand.
 * This method is called from the render thread and in the correct context.
 */
void CanvasRenderer::executeSyncCommand(GlSyncCommand &command)
{
    if (!m_glContext)
        return;

    // Bind the correct render target FBO
    bindCurrentRenderTarget();

    switch (command.id) {
    case CanvasGlCommandQueue::glGetActiveAttrib: {
        QOpenGLShaderProgram *program = m_commandQueue.getProgram(command.i1);
        if (program) {
            GLuint glId = program->programId();
            GLint *retVal = reinterpret_cast<GLint *>(command.returnValue);
            glGetActiveAttrib(glId, GLsizei(command.i2), GLsizei(command.i3),
                              reinterpret_cast<GLsizei *>(&retVal[0]), &retVal[1],
                              reinterpret_cast<GLenum *>(&retVal[2]),
                              reinterpret_cast<char *>(&retVal[3]));
        } else {
            m_glError |= CanvasContext::CANVAS_INVALID_OPERATION;
        }
        break;
    }
    case CanvasGlCommandQueue::glGetActiveUniform: {
        QOpenGLShaderProgram *program = m_commandQueue.getProgram(command.i1);
        if (program) {
            GLuint glId = program->programId();
            GLint *retVal = reinterpret_cast<GLint *>(command.returnValue);
            glGetActiveUniform(glId, GLsizei(command.i2), GLsizei(command.i3),
                               reinterpret_cast<GLsizei *>(&retVal[0]), &retVal[1],
                               reinterpret_cast<GLenum *>(&retVal[2]),
                               reinterpret_cast<char *>(&retVal[3]));
        } else {
            m_glError |= CanvasContext::CANVAS_INVALID_OPERATION;
        }
        break;
    }
    case CanvasGlCommandQueue::glGetAttribLocation: {
        QOpenGLShaderProgram *program = m_commandQueue.getProgram(command.i1);
        if (program) {
            GLint *retVal = reinterpret_cast<GLint *>(command.returnValue);
            *retVal = program->attributeLocation(*command.data);
        } else {
            m_glError |= CanvasContext::CANVAS_INVALID_OPERATION;
        }
        break;
    }
    case CanvasGlCommandQueue::glGetBooleanv: {
        GLboolean *retVal = reinterpret_cast<GLboolean *>(command.returnValue);
        glGetBooleanv(GLenum(command.i1), retVal);
        break;
    }
    case CanvasGlCommandQueue::glGetBufferParameteriv: {
        GLint *retVal = reinterpret_cast<GLint *>(command.returnValue);
        glGetBufferParameteriv(GLenum(command.i1), GLenum(command.i2), retVal);
        break;
    }
    case CanvasGlCommandQueue::glCheckFramebufferStatus: {
        GLenum *value = reinterpret_cast<GLenum *>(command.returnValue);
        *value = glCheckFramebufferStatus(GLenum(command.i1));
        break;
    }
    case CanvasGlCommandQueue::glFinish: {
        glFinish();
        break;
    }
    case CanvasGlCommandQueue::glGetError: {
        int *retVal = reinterpret_cast<int *>(command.returnValue);
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            // Merge any GL errors with internal errors so that we don't lose them
            switch (err) {
            case GL_INVALID_ENUM:
                m_glError |= CanvasContext::CANVAS_INVALID_ENUM;
                break;
            case GL_INVALID_VALUE:
                m_glError |= CanvasContext::CANVAS_INVALID_VALUE;
                break;
            case GL_INVALID_OPERATION:
                m_glError |= CanvasContext::CANVAS_INVALID_OPERATION;
                break;
            case GL_OUT_OF_MEMORY:
                m_glError |= CanvasContext::CANVAS_OUT_OF_MEMORY;
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                m_glError |= CanvasContext::CANVAS_INVALID_FRAMEBUFFER_OPERATION;
                break;
#if defined(GL_STACK_OVERFLOW)
            case GL_STACK_OVERFLOW:
                qCWarning(canvas3dglerrors).nospace() << "CanvasRenderer::" << __FUNCTION__
                                                      << ":GL_STACK_OVERFLOW error ignored";
                break;
#endif
#if defined(GL_STACK_UNDERFLOW)
            case GL_STACK_UNDERFLOW:
                qCWarning(canvas3dglerrors).nospace() << "CanvasRenderer::" << __FUNCTION__
                                                      << ": GL_CANVAS_STACK_UNDERFLOW error ignored";
                break;
#endif
            default:
                break;
            }
        }
        *retVal |= m_glError;
        m_glError = CanvasContext::CANVAS_NO_ERRORS;
        break;
    }
    case CanvasGlCommandQueue::glGetFloatv: {
        GLfloat *retVal = reinterpret_cast<GLfloat *>(command.returnValue);
        glGetFloatv(GLenum(command.i1), retVal);
        break;
    }
    case CanvasGlCommandQueue::glGetFramebufferAttachmentParameteriv: {
        GLint *retVal = reinterpret_cast<GLint *>(command.returnValue);
        glGetFramebufferAttachmentParameteriv(GLenum(command.i1), GLenum(command.i2),
                                              GLenum(command.i3), retVal);
        break;
    }
    case CanvasGlCommandQueue::glGetIntegerv: {
        GLint *retVal = reinterpret_cast<GLint *>(command.returnValue);
        glGetIntegerv(GLenum(command.i1), retVal);
        break;
    }
    case CanvasGlCommandQueue::glGetProgramInfoLog: {
        QOpenGLShaderProgram *program = m_commandQueue.getProgram(command.i1);
        if (program) {
            QString *logStr = reinterpret_cast<QString *>(command.returnValue);
            *logStr = program->log();
        } else {
            m_glError |= CanvasContext::CANVAS_INVALID_OPERATION;
        }
        break;
    }
    case CanvasGlCommandQueue::glGetProgramiv: {
        QOpenGLShaderProgram *program = m_commandQueue.getProgram(command.i1);
        if (program) {
            GLint *retVal = reinterpret_cast<GLint *>(command.returnValue);
            glGetProgramiv(program->programId(), GLenum(command.i2), retVal);
        } else {
            m_glError |= CanvasContext::CANVAS_INVALID_OPERATION;
        }
        break;
    }
    case CanvasGlCommandQueue::glGetRenderbufferParameteriv: {
        GLint *retVal = reinterpret_cast<GLint *>(command.returnValue);
        glGetRenderbufferParameteriv(GLenum(command.i1), GLenum(command.i2), retVal);
        break;
    }
    case CanvasGlCommandQueue::glGetShaderInfoLog: {
        QOpenGLShader *shader = m_commandQueue.getShader(command.i1);
        if (shader) {
            QString *logStr = reinterpret_cast<QString *>(command.returnValue);
            *logStr = shader->log();
        } else {
            m_glError |= CanvasContext::CANVAS_INVALID_OPERATION;
        }
        break;
    }
    case CanvasGlCommandQueue::glGetShaderiv: {
        QOpenGLShader *shader = m_commandQueue.getShader(command.i1);
        if (shader) {
            GLint *retVal = reinterpret_cast<GLint *>(command.returnValue);
            glGetShaderiv(shader->shaderId(), GLenum(command.i2), retVal);
        } else {
            m_glError |= CanvasContext::CANVAS_INVALID_OPERATION;
        }
        break;
    }
    case CanvasGlCommandQueue::glGetShaderPrecisionFormat: {
        GLint *retVal = reinterpret_cast<GLint *>(command.returnValue);
        glGetShaderPrecisionFormat(GLenum(command.i1), GLenum(command.i2), retVal, &retVal[2]);
        break;
    }
    case CanvasGlCommandQueue::glGetTexParameteriv: {
        GLint *retVal = reinterpret_cast<GLint *>(command.returnValue);
        glGetTexParameteriv(GLenum(command.i1), GLenum(command.i2), retVal);
        break;
    }
    case CanvasGlCommandQueue::glGetUniformfv: {
        QOpenGLShaderProgram *program = m_commandQueue.getProgram(command.i1);
        if (program) {
            GLfloat *retVal = reinterpret_cast<GLfloat *>(command.returnValue);
            GLuint programId = program->programId();
            GLint location = m_commandQueue.getGlId(command.i2);
            glGetUniformfv(programId, location, retVal);
        } else {
            m_glError |= CanvasContext::CANVAS_INVALID_OPERATION;
        }
        break;
    }
    case CanvasGlCommandQueue::glGetUniformiv: {
        QOpenGLShaderProgram *program = m_commandQueue.getProgram(command.i1);
        if (program) {
            GLint *retVal = reinterpret_cast<GLint *>(command.returnValue);
            GLuint programId = program->programId();
            GLint location = m_commandQueue.getGlId(command.i2);
            glGetUniformiv(programId, location, retVal);
        } else {
            m_glError |= CanvasContext::CANVAS_INVALID_OPERATION;
        }
        break;
    }
    case CanvasGlCommandQueue::glGetVertexAttribPointerv: {
        GLuint *retVal = reinterpret_cast<GLuint *>(command.returnValue);
        glGetVertexAttribPointerv(GLuint(command.i1), GLenum(command.i2),
                                  reinterpret_cast<GLvoid **>(retVal));
        break;
    }
    case CanvasGlCommandQueue::glGetVertexAttribfv: {
        GLfloat *retVal = reinterpret_cast<GLfloat *>(command.returnValue);
        glGetVertexAttribfv(GLuint(command.i1), GLenum(command.i2), retVal);
        break;
    }
    case CanvasGlCommandQueue::glGetVertexAttribiv: {
        GLint *retVal = reinterpret_cast<GLint *>(command.returnValue);
        glGetVertexAttribiv(GLuint(command.i1), GLenum(command.i2), retVal);
        break;
    }
    case CanvasGlCommandQueue::glGetString: {
        command.returnValue =
                const_cast<GLubyte *>(glGetString(GLenum(command.i1)));
        break;
    }
    case CanvasGlCommandQueue::glIsBuffer: {
        GLboolean *retVal = reinterpret_cast<GLboolean *>(command.returnValue);
        *retVal = glIsBuffer(m_commandQueue.getGlId(command.i1));
        break;
    }
    case CanvasGlCommandQueue::glIsEnabled: {
        GLboolean *retVal = reinterpret_cast<GLboolean *>(command.returnValue);
        *retVal = glIsEnabled(GLenum(command.i1));
        break;
    }
    case CanvasGlCommandQueue::glIsFramebuffer: {
        GLboolean *retVal = reinterpret_cast<GLboolean *>(command.returnValue);
        *retVal = glIsFramebuffer(m_commandQueue.getGlId(command.i1));
        break;
    }
    case CanvasGlCommandQueue::glIsProgram: {
        GLboolean *retVal = reinterpret_cast<GLboolean *>(command.returnValue);
        QOpenGLShaderProgram *program = m_commandQueue.getProgram(command.i1);
        *retVal = program ? glIsProgram(program->programId()) : GL_FALSE;
        break;
    }
    case CanvasGlCommandQueue::glIsRenderbuffer: {
        GLboolean *retVal = reinterpret_cast<GLboolean *>(command.returnValue);
        *retVal = glIsRenderbuffer(m_commandQueue.getGlId(command.i1));
        break;
    }
    case CanvasGlCommandQueue::glIsShader: {
        GLboolean *retVal = reinterpret_cast<GLboolean *>(command.returnValue);
        QOpenGLShader *shader = m_commandQueue.getShader(command.i1);
        *retVal = shader ? glIsShader(shader->shaderId()) : GL_FALSE;
        break;
    }
    case CanvasGlCommandQueue::glIsTexture: {
        GLboolean *retVal = reinterpret_cast<GLboolean *>(command.returnValue);
        *retVal = glIsTexture(m_commandQueue.getGlId(command.i1));
        break;
    }
    case CanvasGlCommandQueue::glReadPixels: {
        // Check if the buffer is antialiased. If it is, we need to blit to the final buffer before
        // reading the value.
        if (m_antialias && !m_currentFramebufferId) {
            GLuint readFbo = resolveMSAAFbo();
            glBindFramebuffer(GL_FRAMEBUFFER, readFbo);
        }

        GLvoid *buf = reinterpret_cast<GLvoid *>(command.returnValue);
        glReadPixels(command.i1, command.i2, GLsizei(command.i3), GLsizei(command.i4),
                     GLenum(command.i5), GLenum(command.i6), buf);
        break;
    }
    case CanvasGlCommandQueue::internalGetUniformType: {
        // The uniform type needs to be known when CanvasContext::getUniform is
        // handled. There is no easy way to determine this, as the active uniform
        // indices do not usually match the uniform locations. We must query
        // active uniforms until we hit the one we want. This is obviously
        // extremely inefficient, but luckily getUniform is not something most
        // users typically need or use.

        QOpenGLShaderProgram *program = m_commandQueue.getProgram(command.i1);
        GLint *retVal = reinterpret_cast<GLint *>(command.returnValue);
        if (program) {
            GLuint programId = program->programId();
            const int maxCharCount = 512;
            GLsizei length;
            GLint size;
            GLenum type;
            char nameBuf[maxCharCount];
            GLint uniformCount = 0;
            glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &uniformCount);
            // Strip any [] from the uniform name, unless part of struct
            QByteArray strippedName = *command.data;
            int idx = strippedName.indexOf('[');
            if (idx >= 0) {
                // Don't truncate if part of struct
                if (strippedName.indexOf('.') == -1)
                    strippedName.truncate(idx);
            }
            for (int i = 0; i < uniformCount; i++) {
                nameBuf[0] = '\0';
                glGetActiveUniform(programId, i, maxCharCount, &length,
                                  &size, &type, nameBuf);
                QByteArray activeName(nameBuf, length);
                idx = activeName.indexOf('[');
                if (idx >= 0) {
                    // Don't truncate if part of struct
                    if (activeName.indexOf('.') == -1)
                        activeName.truncate(idx);
                }

                if (activeName == strippedName) {
                    *retVal = type;
                    break;
                }
            }
        } else {
            *retVal = -1;
        }
        break;
    }
    case CanvasGlCommandQueue::extStateDump: {
        CanvasGLStateDump *dumpObj = reinterpret_cast<CanvasGLStateDump *>(command.returnValue);
        dumpObj->doGLStateDump();
        break;
    }
    default: {
        qWarning() << "CanvasRenderer::" << __FUNCTION__
                   << "Unsupported GL command handled:" << command.id;
        break;
    }
    }

    // Rebind default FBO
    QOpenGLFramebufferObject::bindDefault();

    logGlErrors(__FUNCTION__);
}

/*!
 * \internal
 *
 * This method is called once the all rendering for the frame is done to offscreen FBO.
 * It does the final blitting and swaps the buffers.
 * This method is called from the render thread and in the correct context.
 */
void CanvasRenderer::finalizeTexture()
{
    qCDebug(canvas3drendering).nospace() << "CanvasRenderer::" << __FUNCTION__ << "()";

    // Resolve MSAA
    if (m_antialias)
        resolveMSAAFbo();

    // We need to flush the contents to the FBO before posting the texture,
    // otherwise we might get unexpected results.
    glFlush();
    glFinish();

    // Update frames per second reading after glFinish()
    ++m_fpsFrames;
    if (m_fpsTimer.elapsed() >= 500) {
        qreal avgtime = qreal(m_fpsTimer.restart()) / qreal(m_fpsFrames);
        uint avgFps = qRound(1000.0 / avgtime);
        if (avgFps != m_fps) {
            m_fps = avgFps;
            emit fpsChanged(avgFps);
        }
        m_fpsFrames = 0;
    }

    // Swap
    qSwap(m_renderFbo, m_displayFbo);
    qCDebug(canvas3drendering).nospace() << "CanvasRenderer::" << __FUNCTION__
                                         << " Displaying texture:"
                                         << m_displayFbo->texture()
                                         << " from FBO:" << m_displayFbo->handle();

    if (m_preserveDrawingBuffer) {
        // Copy the content of display fbo to the render fbo
        GLint texBinding2D;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &texBinding2D);

        m_displayFbo->bind();
        glBindTexture(GL_TEXTURE_2D, m_renderFbo->texture());

        glCopyTexImage2D(GL_TEXTURE_2D, 0, m_displayFbo->format().internalTextureFormat(),
                         0, 0, m_fboSize.width(), m_fboSize.height(), 0);

        glBindTexture(GL_TEXTURE_2D, texBinding2D);
    }

    // Notify the render node of new texture parameters
    emit textureReady(m_displayFbo->texture(), m_fboSize);
}

/*!
 * \internal
 *
 * Blits the antialias fbo into the final render fbo.
 * Returns the final render fbo handle.
 * This method is called from the render thread and in the correct context.
 */
GLuint CanvasRenderer::resolveMSAAFbo()
{
    qCDebug(canvas3drendering).nospace() << "CanvasRenderer::" << __FUNCTION__
                                         << " Resolving MSAA from FBO:"
                                         << m_antialiasFbo->handle()
                                         << " to FBO:" << m_renderFbo->handle();
    QOpenGLFramebufferObject::blitFramebuffer(m_renderFbo, m_antialiasFbo);

    return m_renderFbo->handle();
}

QT_CANVAS3D_END_NAMESPACE
QT_END_NAMESPACE
