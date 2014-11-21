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

#include "canvasglstatedump_p.h"
#include "activeinfo3d_p.h"
#include "canvas3d_p.h"
#include "context3d_p.h"
#include "texture3d_p.h"
#include "shader3d_p.h"
#include "program3d_p.h"
#include "buffer3d_p.h"
#include "framebuffer3d_p.h"
#include "renderbuffer3d_p.h"
#include "uniformlocation_p.h"
#include "teximage3d_p.h"
#include "arrayutils_p.h"
#include "shaderprecisionformat_p.h"
#include "enumtostringmap_p.h"
#include "canvas3dcommon_p.h"
#include "contextextensions_p.h"

#include "typedarray/arraybuffer_p.h"
#include "typedarray/int8array_p.h"
#include "typedarray/uint8array_p.h"
#include "typedarray/int16array_p.h"
#include "typedarray/uint16array_p.h"
#include "typedarray/int32array_p.h"
#include "typedarray/uint32array_p.h"
#include "typedarray/float32array_p.h"
#include "typedarray/float64array_p.h"

#include <QtGui/QOpenGLShader>

/*!
 * \qmltype Context3D
 * \since QtCanvas3D 1.0
 * \ingroup qtcanvas3d-qml-types
 * \brief Provides 3D rendering API and context.
 *
 * An uncreatable QML type that provides a WebGL-like API that can be used to draw 3D graphics to
 * the Canvas3D element. You can get it by calling \l{Canvas3D::getContext}{Canvas3D.getContext}
 * method.
 *
 * \sa Canvas3D
 */

CanvasContext::CanvasContext(QOpenGLContext *context, QSurface *surface,
                             int width, int height, bool isES2, QObject *parent) :
    CanvasAbstractObject(parent),
    QOpenGLFunctions(context),
    m_unpackFlipYEnabled(false),
    m_logAllCalls(false),
    m_logAllErrors(true),
    m_glViewportRect(0, 0, width, height),
    m_devicePixelRatio(1.0),
    m_currentProgram(0),
    m_currentArrayBuffer(0),
    m_currentElementArrayBuffer(0),
    m_currentTexture(0),
    m_context(context),
    m_surface(surface),
    m_error(NO_ERROR),
    m_currentFramebuffer(0),
    m_map(EnumToStringMap::newInstance()),
    m_canvas(0),
    m_maxVertexAttribs(0),
    m_isOpenGLES2(isES2),
    m_stateDumpExt(0)
{
    int value = 0;
    glGetIntegerv(MAX_VERTEX_ATTRIBS, &value);
    m_maxVertexAttribs = uint(value);

#ifndef QT_NO_DEBUG
    const GLubyte *version = glGetString(GL_VERSION);
    qDebug() << "Context3D::" << __FUNCTION__
             << "OpenGL version:" << (const char *)version;

    version = glGetString(GL_SHADING_LANGUAGE_VERSION);
    qDebug() << "Context3D::" << __FUNCTION__
             << "GLSL version:" << (const char *)version;

    QString extensions = QString((char *)glGetString(GL_EXTENSIONS));
    QStringList list = extensions.split(" ");
    qDebug() << "Context3D::" << __FUNCTION__
             << "EXTENSIONS: " << list;
#endif
}

/*!
 * \internal
 */
CanvasContext::~CanvasContext()
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__;
    EnumToStringMap::deleteInstance();
}

/*!
 * \qmlproperty bool Context3D::canvas
 * Holds the Canvas3D for the Context3D.
 */
/*!
 * \internal
 */
void CanvasContext::setCanvas(Canvas *canvas)
{
    if (m_canvas != canvas) {
        if (m_canvas) {
            disconnect(m_canvas, &QQuickItem::widthChanged, this, 0);
            disconnect(m_canvas, &QQuickItem::heightChanged, this, 0);
        }


        m_canvas = canvas;
        emit canvasChanged(canvas);

        connect(m_canvas, &QQuickItem::widthChanged,
                this, &CanvasContext::drawingBufferWidthChanged);
        connect(m_canvas, &QQuickItem::heightChanged,
                this, &CanvasContext::drawingBufferHeightChanged);
    }
}

Canvas *CanvasContext::canvas()
{
    return m_canvas;
}

/*!
 * \qmlproperty bool Context3D::drawingBufferWidth
 * Returns the current logical pixel width of the drawing buffer. To get width in physical pixels
 * you need to multiply this with the devicePixelRatio.
 */
uint CanvasContext::drawingBufferWidth()
{
    uint width = 0;
    if (m_canvas)
        width = m_canvas->width();

    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(): " << width;
    return width;
}

/*!
 * \qmlproperty bool Context3D::drawingBufferHeight
 * Returns the current logical pixel height of the drawing buffer. To get height in physical pixels
 * you need to multiply this with the devicePixelRatio.
 */
uint CanvasContext::drawingBufferHeight()
{
    uint height = 0;
    if (m_canvas)
        height = m_canvas->height();

    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(): " << height;
    return height;
}

/*!
 * \internal
 */
QString CanvasContext::glEnumToString(glEnums value) const
{
    return m_map->lookUp(value);
}

/*!
 * \qmlproperty bool Context3D::logAllCalls
 * Specifies if all Context3D method calls (including internal ones) are logged to the console.
 * Defaults to \c{false}.
 * Setting this to \c true may help in debugging your code as you get a full listing of the 3D calls
 * and parameters your code does.
 */
/*!
 * \internal
 */
void CanvasContext::setLogAllCalls(bool logCalls)
{
    if (m_logAllCalls != logCalls) {
        m_logAllCalls = logCalls;
        emit logAllCallsChanged(logCalls);
    }
}

bool CanvasContext::logAllCalls() const
{
    return m_logAllCalls;
}

/*!
 * \qmlproperty bool Context3D::logAllErrors
 * Specifies if all Context3D errors are logged to the console in addition to just setting the
 * getError() return value. Defaults to \c{true}.
 */
/*!
 * \internal
 */
void CanvasContext::setLogAllErrors(bool logErrors)
{
    if (m_logAllErrors != logErrors) {
        m_logAllErrors = logErrors;
        emit logAllErrorsChanged(logErrors);
    }
}

bool CanvasContext::logAllErrors() const
{
    return m_logAllErrors;
}

/*!
 * \internal
 */
void CanvasContext::setContextAttributes(const CanvasContextAttributes &attribs)
{
    m_contextAttributes.setFrom(attribs);
}

/*!
 * \internal
 */
float CanvasContext::devicePixelRatio()
{
    return m_devicePixelRatio;
}

/*!
 * \internal
 */
void CanvasContext::setDevicePixelRatio(float ratio)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__ << "(" << ratio << ")";
    m_devicePixelRatio = ratio;
}

/*!
 * \internal
 */
QRect CanvasContext::glViewportRect() const
{
    return m_glViewportRect;
}

/*!
 * \internal
 */
GLuint CanvasContext::currentFramebuffer()
{
    if (!m_currentFramebuffer)
        return 0;

    return m_currentFramebuffer->id();
}

/*!
 * \qmlmethod ShaderPrecisionFormat Context3D::getShaderPrecisionFormat(glEnums shadertype, glEnums precisiontype)
 * Return a new ShaderPrecisionFormat describing the range and precision for the specified shader
 * numeric format.
 * \a shadertype Type of the shader, either \c Context3D.FRAGMENT_SHADER or
 * \c{Context3D.VERTEX_SHADER}.
 * \a precisiontype Can be \c{Context3D.LOW_FLOAT}, \c{Context3D.MEDIUM_FLOAT},
 * \c{Context3D.HIGH_FLOAT}, \c{Context3D.LOW_INT}, \c{Context3D.MEDIUM_INT} or
 * \c{Context3D.HIGH_INT}.
 *
 * \sa ShaderPrecisionFormat
 */
/*!
 * \internal
 */
CanvasShaderPrecisionFormat *CanvasContext::getShaderPrecisionFormat(glEnums shadertype,
                                                                     glEnums precisiontype)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(shaderType:" << glEnumToString(shadertype)
                                << ", precisiontype:" << glEnumToString(precisiontype)
                                << ")";

    GLint range[2];
    range[0] = 1;
    range[1] = 1;
    GLint precision = 1;

    glGetShaderPrecisionFormat((GLenum)(shadertype), (GLenum)(precisiontype), range, &precision);
    CanvasShaderPrecisionFormat *format = new CanvasShaderPrecisionFormat();
    format->setPrecision(int(precision));
    format->setRangeMin(int(range[0]));
    format->setRangeMax(int(range[1]));
    return format;
}

/*!
 * \qmlmethod bool Context3D::isContextLost()
 * Always returns false.
 */
/*!
 * \internal
 */
bool CanvasContext::isContextLost()
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(): false";
    return false;
}

/*!
 * \qmlmethod ContextAttributes Context3D::getContextAttributes()
 * Returns a copy of the actual context parameters that are used in the current context.
 */
/*!
 * \internal
 */
CanvasContextAttributes *CanvasContext::getContextAttributes()
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "()";

    CanvasContextAttributes *attributes = new CanvasContextAttributes();
    attributes->setAlpha(m_contextAttributes.alpha());
    attributes->setDepth(m_contextAttributes.depth());
    attributes->setStencil(m_contextAttributes.stencil());
    attributes->setAntialias(m_contextAttributes.antialias());
    attributes->setPremultipliedAlpha(m_contextAttributes.premultipliedAlpha());
    attributes->setPreserveDrawingBuffer(m_contextAttributes.preserveDrawingBuffer());
    attributes->setPreferLowPowerToHighPerformance(
                m_contextAttributes.preferLowPowerToHighPerformance());
    attributes->setFailIfMajorPerformanceCaveat(
                m_contextAttributes.failIfMajorPerformanceCaveat());

    // Returning a pointer to QObject that has NO parent set
    // -> V4VM should respect this take ownership
    return attributes;
}

/*!
 * \qmlmethod void Context3D::flush()
 * Indicates to graphics driver that previously sent commands must complete within finite time.
 */
/*!
 * \internal
 */
void CanvasContext::flush()
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "()";
    glFlush();
}

/*!
 * \qmlmethod void Context3D::finish()
 * Forces all previous 3D rendering commands to complete.
 */
/*!
 * \internal
 */
void CanvasContext::finish()
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "()";
    glFinish();
}

/*!
 * \qmlmethod Texture3D Context3D::createTexture()
 * Create a Texture3D object and initialize a name for it as by calling \c{glGenTextures()}.
 */
/*!
 * \internal
 */
CanvasTexture *CanvasContext::createTexture()
{
    CanvasTexture *texture = new CanvasTexture(this);
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "():" << texture;

    // Returning a pointer to QObject that has parent set
    // -> V4VM should respect this and ownership should remain with this class
    return texture;
}

/*!
 * \qmlmethod void Context3D::deleteTexture(Texture3D texture)
 * Deletes the given texture as if by calling \c{glDeleteTextures()}.
 * Calling this method repeatedly on the same object has no side effects.
 * \a texture is the Texture3D to be deleted.
 */
/*!
 * \internal
 */
void CanvasContext::deleteTexture(CanvasTexture *texture)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(texture:" << texture
                                << ")";
    if (texture) {
        texture->del();
    } else {
        m_error = INVALID_VALUE;
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID texture handle:" << texture;
    }
}

/*!
 * \qmlmethod void Context3D::scissor(int x, int y, int width, int height)
 * Defines a rectangle that constrains the drawing.
 * \a x is theleft edge of the rectangle.
 * \a y is the bottom edge of the rectangle.
 * \a width is the width of the rectangle.
 * \a height is the height of the rectangle.
 */
/*!
 * \internal
 */
void CanvasContext::scissor(int x, int y, int width, int height)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(x:" << x
                                << ", y:" << y
                                << ", width:" << width
                                << ", height:" << height
                                << ")";

    glScissor(x, y, width, height);
}

/*!
 * \qmlmethod void Context3D::activeTexture(glEnums texture)
 * Sets the given texture unit as active. Number of texture units is implementation dependent,
 * but must be at least 8. Initially \c Context3D.TEXTURE0 is active.
 * \a texture must be one of \c Context3D.TEXTUREi values where \c i ranges from \c 0 to
 * \c{(Context3D.MAX_COMBINED_TEXTURE_IMAGE_UNITS-1)}.
 */
/*!
 * \internal
 */
void CanvasContext::activeTexture(glEnums texture)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(texture:" << glEnumToString(texture)
                                << ")";
    glActiveTexture(GLenum(texture));
}

/*!
 * \qmlmethod void Context3D::bindTexture(glEnums target, Texture3D texture)
 * Bind a Texture3D to a texturing target.
 * \a target is the target of the active texture unit to which the Texture3D will be bound.
 * Must be either \c{Context3D.TEXTURE_2D} or \c{Context3D.TEXTURE_CUBE_MAP}.
 * \a texture is the Texture3D to be bound.
 */
/*!
 * \internal
 */
void CanvasContext::bindTexture(glEnums target, CanvasTexture *texture)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(target:" << glEnumToString(target)
                                << ", texture:" << texture
                                << ")";
    m_currentTexture = texture;
    if (texture) {
        if (!texture->isAlive()) {
            if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                         << ": Trying to bind deleted texture object";
            return;
        }
        m_currentTexture->bind(target);
    } else {
        glBindTexture(GLenum(target), 0);
    }
}

/*!
 * \qmlmethod void Context3D::generateMipmap(glEnums target)
 * Generates a complete set of mipmaps for a texture object of the currently active texture unit.
 * \a target defines the texture target to which the texture object is bound whose mipmaps will be
 * generated. Must be either \c{Context3D.TEXTURE_2D} or \c{Context3D.TEXTURE_CUBE_MAP}.
 */
/*!
 * \internal
 */
void CanvasContext::generateMipmap(glEnums target)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(target:" << glEnumToString(target)
                                << ")";
    if (!m_currentTexture) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_OPERATION No current texture bound";
        m_error = INVALID_OPERATION;
    } else if (!m_currentTexture->isAlive()) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_OPERATION Currently bound texture is deleted";
        m_error = INVALID_OPERATION;
    } else {
        glGenerateMipmap(target);
    }
}

/*!
 * \qmlmethod bool Context3D::isTexture(Object anyObject)
 * Returns true if the given object is a valid Texture3D object.
 * \a anyObject is the object that is to be verified as a valid texture.
 */
/*!
 * \internal
 */
bool CanvasContext::isTexture(QObject *anyObject)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(anyObject:" << anyObject
                                << ")";

    if (!anyObject)
        return false;

    QString className = QString(anyObject->metaObject()->className());
    if (className != "CanvasTexture")
        return false;

    CanvasTexture *texture = static_cast<CanvasTexture *>(anyObject);
    if (!texture->isAlive())
        return false;
    return glIsTexture(texture->textureId());
}

/*!
 * \qmlmethod void Context3D::compressedTexImage2D(glEnums target, int level, glEnums internalformat, int width, int height, int border, TypedArray pixels)
 * Not supported, \c{Context3D.INVALID_OPERATION} is generated when called.
 * \a target, \a level, \a internalformat, \a width, \a height, \a border and \a pixels are ignored.
 */
/*!
 * \internal
 */
void CanvasContext::compressedTexImage2D(glEnums target, int level, glEnums internalformat,
                                         int width, int height, int border,
                                         CanvasTypedArray *pixels)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(target:" << glEnumToString(target)
                                << ", level:" << level
                                << ", internalformat:" << glEnumToString(internalformat)
                                << ", width:" << width
                                << ", height:" << height
                                << ", border:" << border
                                << ", pixels:" << pixels
                                << ")";

    if (!m_currentTexture) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_OPERATION No current texture bound";
        m_error = INVALID_OPERATION;
    } else if (!m_currentTexture->isAlive()) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_OPERATION Currently bound texture is deleted";
        m_error = INVALID_OPERATION;
    } else {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_ENUM API doesn't support compressed images";
        m_error = INVALID_ENUM;
    }
}

/*!
 * \qmlmethod void Context3D::compressedTexSubImage2D(glEnums target, int level, int xoffset, int yoffset, int width, int height, glEnums format, TypedArray pixels)
 * Not supported, \c{Context3D.INVALID_OPERATION} is generated when called.
 * \a target, \a level, \a xoffset, \a yoffset, \a width, \a height, \a format and \a pixels are
 * ignored.
 */
/*!
 * \internal
 */
void CanvasContext::compressedTexSubImage2D(glEnums target, int level,
                                            int xoffset, int yoffset,
                                            int width, int height,
                                            glEnums format,
                                            CanvasTypedArray *pixels)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(target:" << glEnumToString(target)
                                << ", level:" << level
                                << ", xoffset:" << xoffset
                                << ", yoffset:" << yoffset
                                << ", width:" << width
                                << ", height:" << height
                                << ", format:" << glEnumToString(format)
                                << ", pixels:" << pixels
                                << ")";

    if (!pixels) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_OPERATION No current texture bound";
        m_error = INVALID_OPERATION;
    }

    if (!m_currentTexture) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_OPERATION No current texture bound";
        m_error = INVALID_OPERATION;
    } else if (!m_currentTexture->isAlive()) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_OPERATION Currently bound texture is deleted";
        m_error = INVALID_OPERATION;
    } else {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_ENUM API doesn't support compressed images";
        m_error = INVALID_ENUM;
    }
}

/*!
 * \qmlmethod void Context3D::copyTexImage2D(glEnums target, int level, glEnums internalformat, int x, int y, int width, int height, int border)
 * Copies pixels into currently bound 2D texture.
 * \a target specifies the target texture of the active texture unit. Must be \c{Context3D.TEXTURE_2D},
 * \c{Context3D.TEXTURE_CUBE_MAP_POSITIVE_X}, \c{Context3D.TEXTURE_CUBE_MAP_NEGATIVE_X},
 * \c{Context3D.TEXTURE_CUBE_MAP_POSITIVE_Y}, \c{Context3D.TEXTURE_CUBE_MAP_NEGATIVE_Y},
 * \c{Context3D.TEXTURE_CUBE_MAP_POSITIVE_Z}, or \c{Context3D.TEXTURE_CUBE_MAP_NEGATIVE_Z}.
 * \a level specifies the level of detail number. Level \c 0 is the base image level. Level \c n is
 * the \c{n}th mipmap reduction image.
 * \a internalformat specifies the internal format of the texture. Must be \c{Context3D.ALPHA},
 * \c{Context3D.LUMINANCE}, \c{Context3D.LUMINANCE_ALPHA}, \c{Context3D.RGB} or \c{Context3D.RGBA}.
 * \a x specifies the window coordinate of the left edge of the rectangular region of pixels to be
 * copied.
 * \a y specifies the window coordinate of the lower edge of the rectangular region of pixels to be
 * copied.
 * \a width specifies the width of the texture image. All implementations will support 2D texture
 * images that are at least 64 texels wide and cube-mapped texture images that are at least 16
 * texels wide.
 * \a height specifies the height of the texture image. All implementations will support 2D texture
 * images that are at least 64 texels high and cube-mapped texture images that are at least 16
 * texels high.
 * \a border must be \c{0}.
 */
/*!
 * \internal
 */
void CanvasContext::copyTexImage2D(glEnums target, int level, glEnums internalformat,
                                   int x, int y, int width, int height,
                                   int border)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(target:" << glEnumToString(target)
                                << ", level:" << level
                                << ", internalformat:" << glEnumToString(internalformat)
                                << ", x:" << x
                                << ", y:" << y
                                << ", width:" << width
                                << ", height:" << height
                                << ", border:" << border
                                << ")";

    if (!m_currentTexture) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_OPERATION No current texture bound";
        m_error = INVALID_OPERATION;
    } else if (!m_currentTexture->isAlive()) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_OPERATION Currently bound texture is deleted";
        m_error = INVALID_OPERATION;
    } else {
        glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
    }
}

/*!
 * \qmlmethod void Context3D::copyTexSubImage2D(glEnums target, int level, int xoffset, int yoffset, int x, int y, int width, int height)
 * Copies to into a currently bound 2D texture subimage.
 * \a target specifies the target texture of the active texture unit. Must be
 * \c{Context3D.TEXTURE_2D},
 * \c{Context3D.TEXTURE_CUBE_MAP_POSITIVE_X}, \c{Context3D.TEXTURE_CUBE_MAP_NEGATIVE_X},
 * \c{Context3D.TEXTURE_CUBE_MAP_POSITIVE_Y}, \c{Context3D.TEXTURE_CUBE_MAP_NEGATIVE_Y},
 * \c{Context3D.TEXTURE_CUBE_MAP_POSITIVE_Z}, or \c{Context3D.TEXTURE_CUBE_MAP_NEGATIVE_Z}.
 * \a level specifies the level of detail number. Level \c 0 is the base image level. Level \c n is
 * the \c{n}th mipmap reduction image.
 * \a xoffset specifies the texel offset in the x direction within the texture array.
 * \a yoffset specifies the texel offset in the y direction within the texture array.
 * \a x specifies the window coordinate of the left edge of the rectangular region of pixels to be
 * copied.
 * \a y specifies the window coordinate of the lower edge of the rectangular region of pixels to be
 * copied.
 * \a width specifies the width of the texture subimage.
 * \a height specifies the height of the texture subimage.
 */
/*!
 * \internal
 */
void CanvasContext::copyTexSubImage2D(glEnums target, int level,
                                      int xoffset, int yoffset,
                                      int x, int y,
                                      int width, int height)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(target:" << glEnumToString(target)
                                << ", level:" << level
                                << ", xoffset:" << xoffset
                                << ", yoffset:" << yoffset
                                << ", x:" << x
                                << ", y:" << y
                                << ", width:" << width
                                << ", height:" << height
                                << ")";

    if (!m_currentTexture) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_OPERATION No current texture bound";
        m_error = INVALID_OPERATION;
    } else if (!m_currentTexture->isAlive()) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_OPERATION Currently bound texture is deleted";
        m_error = INVALID_OPERATION;
    } else {
        copyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
    }
}

/*!
 * \qmlmethod void Context3D::texImage2D(glEnums target, int level, glEnums internalformat, int width, int height, int border, glEnums format, glEnums type, TypedArray pixels)
 * Specify a 2D texture image.
 * \a target specifies the target texture of the active texture unit. Must be one of:
 * \c{Context3D.TEXTURE_2D},
 * \c{Context3D.TEXTURE_CUBE_MAP_POSITIVE_X}, \c{Context3D.TEXTURE_CUBE_MAP_NEGATIVE_X},
 * \c{Context3D.TEXTURE_CUBE_MAP_POSITIVE_Y}, \c{Context3D.TEXTURE_CUBE_MAP_NEGATIVE_Y},
 * \c{Context3D.TEXTURE_CUBE_MAP_POSITIVE_Z}, or \c{Context3D.TEXTURE_CUBE_MAP_NEGATIVE_Z}.
 * \a level specifies the level of detail number. Level \c 0 is the base image level. Level \c n is
 * the \c{n}th mipmap reduction image.
 * \a internalformat specifies the internal format of the texture. Must be \c{Context3D.ALPHA},
 * \c{Context3D.LUMINANCE}, \c{Context3D.LUMINANCE_ALPHA}, \c{Context3D.RGB} or \c{Context3D.RGBA}.
 * \a width specifies the width of the texture image. All implementations will support 2D texture
 * images that are at least 64 texels wide and cube-mapped texture images that are at least 16
 * texels wide.
 * \a height specifies the height of the texture image. All implementations will support 2D texture
 * images that are at least 64 texels high and cube-mapped texture images that are at least 16
 * texels high.
 * \a border must be \c{0}.
 * \a format specifies the format of the texel data given in \a pixels, must match the value
 * of \a internalFormat.
 * \a type specifies the data type of the data given in \a pixels, must match the TypedArray type
 * of \a pixels. Must be \c{Context3D.UNSIGNED_BYTE}, \c{Context3D.UNSIGNED_SHORT_5_6_5},
 * \c{Context3D.UNSIGNED_SHORT_4_4_4_4} or \c{Context3D.UNSIGNED_SHORT_5_5_5_1}.
 * \a pixels specifies the TypedArray containing the image data. If pixels is \c{null}, a buffer
 * of sufficient size initialized to 0 is passed.
 */
/*!
 * \internal
 */
void CanvasContext::texImage2D(glEnums target, int level, glEnums internalformat,
                               int width, int height, int border,
                               glEnums format, glEnums type,
                               CanvasTypedArray *pixels)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(target:" << glEnumToString(target)
                                << ", level:" << level
                                << ", internalformat:" << glEnumToString(internalformat)
                                << ", width:" << width
                                << ", height:" << height
                                << ", border:" << border
                                << ", format:" << glEnumToString(format)
                                << ", type:" << glEnumToString(type)
                                << ", pixels:" << pixels
                                << ")";
    if (!m_currentTexture) {
        if (m_logAllErrors) qDebug() << "Context3D::"
                                     << __FUNCTION__
                                     << ":INVALID_OPERATION No current texture bound";
        m_error = INVALID_OPERATION;
        return;
    } else if (!m_currentTexture->isAlive()) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_OPERATION Currently bound texture is deleted";
        m_error = INVALID_OPERATION;
        return;
    }

    bool deleteTempPixels = false;
    if (!pixels) {
        deleteTempPixels = true;
        if (type == UNSIGNED_BYTE )
            pixels = new CanvasUint8Array(getSufficientSize(internalformat, width, height));
        else
            pixels = new CanvasUint16Array(getSufficientSize(internalformat, width, height));
    }

    QString className(pixels->metaObject()->className());

    int bytesPerPixel = 0;
    uchar *srcData = 0;
    uchar *unpackedData = 0;
    switch (type) {
    case UNSIGNED_BYTE: {
        if (className != "CanvasUint8Array") {
            if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                         << ":INVALID_OPERATION Expected Uint8Array, received "
                                         << pixels->metaObject()->className();
            m_error = INVALID_OPERATION;
            return;
        }

        switch (format) {
        case ALPHA:
            bytesPerPixel = 1;
            break;
        case RGB:
            bytesPerPixel = 3;
            break;
        case RGBA:
            bytesPerPixel = 4;
            break;
        case LUMINANCE:
            bytesPerPixel = 1;
            break;
        case LUMINANCE_ALPHA:
            bytesPerPixel = 2;
            break;
        default:
            break;
        }

        if (bytesPerPixel == 0) {
            m_error = INVALID_ENUM;
            if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                         << ":INVALID_ENUM Invalid format supplied "
                                         << glEnumToString(format);
            return;
        }

        srcData = static_cast<CanvasUint8Array *>(pixels)->rawDataCptr();
        unpackedData = unpackPixels(srcData, false, bytesPerPixel, width, height);
        glTexImage2D(target, level, internalformat, width, height,
                     border, format, type, unpackedData);
    }
        break;
    case UNSIGNED_SHORT_4_4_4_4:
    case UNSIGNED_SHORT_5_6_5:
    case UNSIGNED_SHORT_5_5_5_1: {
        if (className != "CanvasUint16Array") {
            if (m_logAllErrors) qDebug() << "Context3D::"
                                         << __FUNCTION__
                                         << ":INVALID_OPERATION Expected Uint16Array, received "
                                         << pixels->metaObject()->className();
            m_error = INVALID_OPERATION;
            return;
        }
        srcData = static_cast<CanvasUint16Array *>(pixels)->rawDataCptr();
        unpackedData = unpackPixels(srcData, false, 2, width, height);
        glTexImage2D(target, level, internalformat, width, height,
                     border, format, type, unpackedData);
    }
        break;
    default:
        if (m_logAllErrors) qDebug() << "Context3D::"
                                     << __FUNCTION__
                                     << ":INVALID_ENUM Invalid type enum";
        m_error = INVALID_ENUM;
        break;
    }

    // Delete temp data
    if (unpackedData != srcData)
        delete unpackedData;

    if (deleteTempPixels)
        delete pixels;
}


/*!
 * \qmlmethod void Context3D::texSubImage2D(glEnums target, int level, int xoffset, int yoffset, int width, int height, glEnums format, glEnums type, TypedArray pixels)
 * Specify a 2D texture subimage.
 * \a target Target texture of the active texture unit. Must be \c{Context3D.TEXTURE_2D},
 * \c{Context3D.TEXTURE_CUBE_MAP_POSITIVE_X}, \c{Context3D.TEXTURE_CUBE_MAP__NEGATIVE_X},
 * \c{Context3D.TEXTURE_CUBE_MAP_POSITIVE_Y}, \c{Context3D.TEXTURE_CUBE_MAP__NEGATIVE_Y},
 * \c{Context3D.TEXTURE_CUBE_MAP_POSITIVE_Z}, or \c{Context3D.TEXTURE_CUBE_MAP__NEGATIVE_Z}.
 * \a level Level of detail number. Level \c 0 is the base image level. Level \c n is the \c{n}th
 * mipmap reduction image.
 * \a xoffset Specifies a texel offset in the x direction within the texture array.
 * \a yoffset Specifies a texel offset in the y direction within the texture array.
 * \a width Width of the texture subimage.
 * \a height Height of the texture subimage.
 * \a format Format of the texel data given in \a pixels, must match the value
 * of \a internalFormat.
 * \a type Data type of the data given in \a pixels, must match the TypedArray type
 * of \a pixels. Must be \c{Context3D.UNSIGNED_BYTE}, \c{Context3D.UNSIGNED_SHORT_5_6_5},
 * \c{Context3D.UNSIGNED_SHORT_4_4_4_4} or \c{Context3D.UNSIGNED_SHORT_5_5_5_1}.
 * \a pixels TypedArray containing the image data. If pixels is \c null, a buffer of
 * sufficient size initialized to 0 is passed.
 */
/*!
 * \internal
 */
void CanvasContext::texSubImage2D(glEnums target, int level,
                                  int xoffset, int yoffset,
                                  int width, int height,
                                  glEnums format, glEnums type,
                                  CanvasTypedArray *pixels)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(target:" << glEnumToString(target)
                                << ", level:" << level
                                << ", xoffset:" << xoffset
                                << ", yoffset:" << yoffset
                                << ", width:" << width
                                << ", height:" << height
                                << ", format:" << glEnumToString(format)
                                << ", type:" << glEnumToString(type)
                                << ", pixels:" << pixels
                                << ")";
    if (!m_currentTexture) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_OPERATION No current texture bound";
        m_error = INVALID_OPERATION;
        return;
    } else if (!m_currentTexture->isAlive()) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_OPERATION Currently bound texture is deleted";
        m_error = INVALID_OPERATION;
        return;
    }

    if (!pixels) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_VALUE pixels was null";
        m_error = INVALID_VALUE;
        return;
    }

    QString className(pixels->metaObject()->className());

    uchar *srcData = 0;
    uchar *unpackedData = 0;
    int bytesPerPixel = 0;
    switch (type) {
    case UNSIGNED_BYTE: {
        if (className != "CanvasUint8Array") {
            if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                         << ":INVALID_OPERATION Expected Uint8Array, received "
                                         << pixels->metaObject()->className();
            m_error = INVALID_OPERATION;
            return;
        }
        switch (format) {
        case ALPHA:
            bytesPerPixel = 1;
            break;
        case RGB:
            bytesPerPixel = 3;
            break;
        case RGBA:
            bytesPerPixel = 4;
            break;
        case LUMINANCE:
            bytesPerPixel = 1;
            break;
        case LUMINANCE_ALPHA:
            bytesPerPixel = 2;
            break;
        default:
            break;
        }

        if (bytesPerPixel == 0) {
            m_error = INVALID_ENUM;
            if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                         << ":INVALID_ENUM Invalid format supplied "
                                         << glEnumToString(format);
            return;
        }

        srcData = static_cast<CanvasUint8Array *>(pixels)->rawDataCptr();
        unpackedData = unpackPixels(srcData, false, bytesPerPixel, width, height);
        glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, unpackedData);
    }
        break;
    case UNSIGNED_SHORT_4_4_4_4:
    case UNSIGNED_SHORT_5_6_5:
    case UNSIGNED_SHORT_5_5_5_1: {
        if (className != "CanvasUint16Array") {
            if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                         << ":INVALID_OPERATION Expected Uint16Array, received "
                                         << pixels->metaObject()->className();

            m_error = INVALID_OPERATION;
            return;
        }
        srcData = static_cast<CanvasUint16Array *>(pixels)->rawDataCptr();
        unpackedData = unpackPixels(srcData, false, 2, width, height);
        glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, unpackedData);
    }
        break;
    default:
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_ENUM Invalid type enum";
        m_error = INVALID_ENUM;
        break;
    }

    // Delete temp data
    if (unpackedData != srcData)
        delete unpackedData;
}

/*!
 * \internal
 */
uchar* CanvasContext::unpackPixels(uchar *srcData, bool useSrcDataAsDst,
                                   int bytesPerPixel, int width, int height)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(srcData:" << srcData
                                << ", useSrcDataAsDst:" << useSrcDataAsDst
                                << ", bytesPerPixel:" << bytesPerPixel
                                << ", width:" << width
                                << ", height:" << height
                                << ")";

    // Check if no processing is needed
    if (!m_unpackFlipYEnabled || srcData == 0 || width == 0 || height == 0 || bytesPerPixel == 0)
        return srcData;

    uchar *unpackedData = srcData;
    int bytesPerRow = width * bytesPerPixel;
    if (m_unpackFlipYEnabled) {
        if (useSrcDataAsDst) {
            uchar *row = new uchar[width*bytesPerPixel];
            for (int y = 0; y < height; y++) {
                memcpy(row,
                       srcData + y * bytesPerRow,
                       bytesPerRow);
                memcpy(srcData + y * bytesPerRow,
                       srcData + (height - y - 1) * bytesPerRow,
                       bytesPerRow);
                memcpy(srcData + (height - y - 1) * bytesPerRow,
                       row,
                       bytesPerRow);
            }
        } else {
            unpackedData = new uchar[height * bytesPerRow];
            for (int y = 0; y < height; y++) {
                memcpy(unpackedData + (height - y - 1) * bytesPerRow,
                       srcData + y * bytesPerRow,
                       bytesPerRow);
            }
        }
    }

    return unpackedData;
}

/*!
 * \qmlmethod void Context3D::texImage2D(glEnums target, int level, glEnums internalformat, glEnums format, glEnums type, TextureImage image)
 * Uploads the given TextureImage element to the currently bound Texture3D.
 * \a target Target texture of the active texture unit. Must be \c{Context3D.TEXTURE_2D},
 * \c{Context3D.TEXTURE_CUBE_MAP_POSITIVE_X}, \c{Context3D.TEXTURE_CUBE_MAP__NEGATIVE_X},
 * \c{Context3D.TEXTURE_CUBE_MAP_POSITIVE_Y}, \c{Context3D.TEXTURE_CUBE_MAP__NEGATIVE_Y},
 * \c{Context3D.TEXTURE_CUBE_MAP_POSITIVE_Z}, or \c{Context3D.TEXTURE_CUBE_MAP__NEGATIVE_Z}.
 * \a level Level of detail number. Level \c 0 is the base image level. Level \c n is the \c{n}th
 * mipmap reduction image.
 * \a internalformat Internal format of the texture, conceptually the given image is first
 * converted to this format, then uploaded. Must be \c{Context3D.ALPHA}, \c{Context3D.LUMINANCE},
 * \c{Context3D.LUMINANCE_ALPHA}, \c{Context3D.RGB} or \c{Context3D.RGBA}.
 * \a format Format of the texture, must match the value of \a internalFormat.
 * \a type Type of the data, conceptually the given image is first converted to this type, then
 * uploaded. Must be \c{Context3D.UNSIGNED_BYTE}, \c{Context3D.UNSIGNED_SHORT_5_6_5},
 * \c{Context3D.UNSIGNED_SHORT_4_4_4_4} or \c{Context3D.UNSIGNED_SHORT_5_5_5_1}.
 * \a image A complete TextureImage loaded using the TextureImageLoader.
 */
/*!
 * \internal
 */
void CanvasContext::texImage2D(glEnums target, int level, glEnums internalformat,
                               glEnums format, glEnums type, CanvasTextureImage *image)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(target:" << glEnumToString(target)
                                << ", level:" << level
                                << ", internalformat:" << glEnumToString(internalformat)
                                << ", format:" << glEnumToString(format)
                                << ", type:" << glEnumToString(type)
                                << ", image:" << image
                                << ")";

    if (!m_currentTexture) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_OPERATION No current texture bound";
        m_error = INVALID_OPERATION;
        return;
    } else if (!m_currentTexture->isAlive()) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_OPERATION Currently bound texture is deleted";
        m_error = INVALID_OPERATION;
        return;
    }

    uchar *pixels = 0;
    switch (type) {
    case UNSIGNED_BYTE:
        pixels = image->convertToFormat(type, m_unpackFlipYEnabled);
        break;
    case UNSIGNED_SHORT_5_6_5:
    case UNSIGNED_SHORT_4_4_4_4:
    case UNSIGNED_SHORT_5_5_5_1:
        pixels = image->convertToFormat(type, m_unpackFlipYEnabled);
        break;
    default:
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_ENUM Invalid type enum";
        m_error = INVALID_ENUM;
        return;
    }

    if (pixels == 0) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":Conversion of pixels to format failed.";
        return;
    }

    glTexImage2D(target, level, internalformat, image->width(), image->height(), 0, format, type,
                 pixels);
}

/*!
 * \qmlmethod void Context3D::texSubImage2D(glEnums target, int level, int xoffset, int yoffset, glEnums format, glEnums type, TextureImage image)
 * Uploads the given TextureImage element to the currently bound Texture3D.
 * \a target specifies the target texture of the active texture unit. Must be
 * \c{Context3D.TEXTURE_2D}, \c{Context3D.TEXTURE_CUBE_MAP_POSITIVE_X},
 * \c{Context3D.TEXTURE_CUBE_MAP__NEGATIVE_X}, \c{Context3D.TEXTURE_CUBE_MAP_POSITIVE_Y},
 * \c{Context3D.TEXTURE_CUBE_MAP__NEGATIVE_Y}, \c{Context3D.TEXTURE_CUBE_MAP_POSITIVE_Z}, or
 * \c{Context3D.TEXTURE_CUBE_MAP__NEGATIVE_Z}.
 * \a level Level of detail number. Level \c 0 is the base image level. Level \c n is the \c{n}th
 * mipmap reduction image.
 * \a internalformat Internal format of the texture, conceptually the given image is first
 * converted to this format, then uploaded. Must be \c{Context3D.ALPHA}, \c{Context3D.LUMINANCE},
 * \c{Context3D.LUMINANCE_ALPHA}, \c{Context3D.RGB} or \c{Context3D.RGBA}.
 * \a format Format of the texture, must match the value of \a internalFormat.
 * \a type Type of the data, conceptually the given image is first converted to this type, then
 * uploaded. Must be \c{Context3D.UNSIGNED_BYTE}, \c{Context3D.UNSIGNED_SHORT_5_6_5},
 * \c{Context3D.UNSIGNED_SHORT_4_4_4_4} or \c{Context3D.UNSIGNED_SHORT_5_5_5_1}.
 * \a image A complete TextureImage loaded using the TextureImageLoader.
 */
/*!
 * \internal
 */
void CanvasContext::texSubImage2D(glEnums target, int level,
                                  int xoffset, int yoffset,
                                  glEnums format, glEnums type, CanvasTextureImage *image)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "( target:" << glEnumToString(target)
                                << ", level:" << level
                                << ", xoffset:" << xoffset
                                << ", yoffset:" << yoffset
                                << ", format:" << glEnumToString(format)
                                << ", type:" << glEnumToString(type)
                                << ", image:" << image
                                << ")";

    if (!m_currentTexture) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_OPERATION No current texture bound";
        m_error = INVALID_OPERATION;
        return;
    } else if (!m_currentTexture->isAlive()) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_OPERATION Currently bound texture is deleted";
        m_error = INVALID_OPERATION;
        return;
    }
    if (!image) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_VALUE image was null";
        m_error = INVALID_VALUE;
        return;
    }

    uchar *pixels = 0;
    switch (type) {
    case UNSIGNED_BYTE:
        pixels = image->convertToFormat(type, m_unpackFlipYEnabled);
        break;
    case UNSIGNED_SHORT_5_6_5:
    case UNSIGNED_SHORT_4_4_4_4:
    case UNSIGNED_SHORT_5_5_5_1:
        pixels = image->convertToFormat(type, m_unpackFlipYEnabled);
        break;
    default:
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_ENUM Invalid type enum";
        m_error = INVALID_ENUM;
        return;
    }

    if (pixels == 0) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":Conversion of pixels to format failed.";
        return;
    }

    glTexSubImage2D(target, level, xoffset, yoffset, image->width(), image->height(), format,
                    type, pixels);
}

/*!
 * \qmlmethod void Context3D::texParameterf(glEnums target, glEnums pname, float param)
 * Sets texture parameters.
 * \a target specifies the target texture of the active texture unit. Must be
 * \c{Context3D.TEXTURE_2D} or \c{Context3D.TEXTURE_CUBE_MAP}.
 * \a pname specifies the symbolic name of a texture parameter. pname can be
 * \c{Context3D.TEXTURE_MIN_FILTER}, \c{Context3D.TEXTURE_MAG_FILTER}, \c{Context3D.TEXTURE_WRAP_S} or
 * \c{Context3D.TEXTURE_WRAP_T}.
 * \a param specifies the new float value to be set to \a pname
 */
/*!
 * \internal
 */
void CanvasContext::texParameterf(glEnums target, glEnums pname, float param)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "( target:" << glEnumToString(target)
                                << ", pname:" << glEnumToString(pname)
                                << ", param:" << param
                                << ")";

    if (!m_currentTexture) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_OPERATION No current texture bound";
        m_error = INVALID_OPERATION;
        return;
    } else if (!m_currentTexture->isAlive()) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_OPERATION Currently bound texture is deleted";
        m_error = INVALID_OPERATION;
        return;
    }

    glTexParameterf(GLenum(target), GLenum(pname), GLfloat(param));
}

/*!
 * \qmlmethod void Context3D::texParameteri(glEnums target, glEnums pname, float param)
 * Sets texture parameters.
 * \a target specifies the target texture of the active texture unit. Must be
 * \c{Context3D.TEXTURE_2D} or \c{Context3D.TEXTURE_CUBE_MAP}.
 * \a pname specifies the symbolic name of a texture parameter. pname can be
 * \c{Context3D.TEXTURE_MIN_FILTER}, \c{Context3D.TEXTURE_MAG_FILTER}, \c{Context3D.TEXTURE_WRAP_S} or
 * \c{Context3D.TEXTURE_WRAP_T}.
 * \a param specifies the new int value to be set to \a pname
 */
/*!
 * \internal
 */
void CanvasContext::texParameteri(glEnums target, glEnums pname, int param)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(target:" << glEnumToString(target)
                                << ", pname:" << glEnumToString(pname)
                                << ", param:" << glEnumToString(glEnums(param))
                                << ")";
    if (!m_currentTexture) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_OPERATION No current texture bound";
        m_error = INVALID_OPERATION;
        return;
    } else if (!m_currentTexture->isAlive()) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_OPERATION Currently bound texture is deleted";
        m_error = INVALID_OPERATION;
        return;
    }

    glTexParameteri(GLenum(target), GLenum(pname), GLint(param));
}

/*!
 * \internal
 */
int CanvasContext::getSufficientSize(glEnums internalFormat, int width, int height)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "( internalFormat:" << glEnumToString(internalFormat)
                                << " , width:" << width
                                << ", height:" << height
                                << ")";
    int bytesPerPixel = 0;
    switch (internalFormat) {
    case UNSIGNED_BYTE:
        bytesPerPixel = 4;
        break;
    case UNSIGNED_SHORT_5_6_5:
    case UNSIGNED_SHORT_4_4_4_4:
    case UNSIGNED_SHORT_5_5_5_1:
        bytesPerPixel = 2;
        break;
    default:
        break;
    }

    return width * height * bytesPerPixel;
}

/*!
 * \qmlmethod FrameBuffer3D Context3D::createFramebuffer()
 * Returns a created FrameBuffer3D object that is initialized with a framebuffer object name as
 * if by calling \c{glGenFramebuffers()}.
 */
/*!
 * \internal
 */
CanvasFrameBuffer *CanvasContext::createFramebuffer()
{
    CanvasFrameBuffer *framebuffer = new CanvasFrameBuffer(this);
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << ":" << framebuffer;

    // Returning a pointer to QObject that has parent set
    // -> V4VM should respect this and ownership should remain with this class
    return framebuffer;
}

/*!
 * \qmlmethod void Context3D::bindFramebuffer(glEnums target, FrameBuffer3D framebuffer)
 * Binds the given \a framebuffer object to the given \a target.
 * \a target must be \c{Context3D.FRAMEBUFFER}.
 */
/*!
 * \internal
 */
void CanvasContext::bindFramebuffer(glEnums target, CanvasFrameBuffer* framebuffer)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(target:" << glEnumToString(target)
                                << ", framebuffer:" << framebuffer << ")";

    if (target != FRAMEBUFFER) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << "(): INVALID_ENUM bind target, must be FRAMEBUFFER";
        m_error = INVALID_ENUM;
        return;
    }

    if (framebuffer) {
        m_currentFramebuffer = framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->id());
    } else {
        m_currentFramebuffer = 0;
        glBindFramebuffer(GL_FRAMEBUFFER, m_canvas->drawFBOHandle());
    }
}

/*!
 * \qmlmethod Context3D::glEnums Context3D::checkFramebufferStatus(glEnums target)
 * Returns the completeness status of the framebuffer object.
 * \a target must be \c{Context3D.FRAMEBUFFER}.
 *
 */
/*!
 * \internal
 */
CanvasContext::glEnums CanvasContext::checkFramebufferStatus(glEnums target)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(target:" << glEnumToString(target)
                                << ")";
    if (target != FRAMEBUFFER) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ": INVALID_ENUM bind target, must be FRAMEBUFFER";
        m_error = INVALID_ENUM;
        return FRAMEBUFFER_UNSUPPORTED;
    }

    if (m_currentFramebuffer) {
        return glEnums(glCheckFramebufferStatus(GL_FRAMEBUFFER));
    } else {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ": INVALID_OPERATION no current framebuffer bound";
        m_error = INVALID_OPERATION;
        return FRAMEBUFFER_UNSUPPORTED;
    }
}

/*!
 * \qmlmethod void Context3D::framebufferRenderbuffer(glEnums target, glEnums attachment, glEnums renderbuffertarget, RenderBuffer3D renderbuffer)
 * Attaches the given \a renderbuffer object to the \a attachment point of the current framebuffer
 * object.
 * \a target must be \c{Context3D.FRAMEBUFFER}. \a renderbuffertarget must
 * be \c{Context3D.RENDERBUFFER}.
 */
/*!
 * \internal
 */
void CanvasContext::framebufferRenderbuffer(glEnums target, glEnums attachment,
                                            glEnums renderbuffertarget,
                                            CanvasRenderBuffer *renderbuffer)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(target:" << glEnumToString(target)
                                << "attachment:" << glEnumToString(attachment)
                                << "renderbuffertarget:" << glEnumToString(renderbuffertarget)
                                << ", renderbuffer:" << renderbuffer
                                << ")";

    if (target != FRAMEBUFFER) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ": INVALID_ENUM  bind target, must be FRAMEBUFFER";
        m_error = INVALID_ENUM;
        return;
    }

    if (!m_currentFramebuffer) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << "(): INVALID_OPERATION no current framebuffer bound";
        m_error = INVALID_OPERATION;
        return;
    }

    if (attachment != COLOR_ATTACHMENT0 && attachment != DEPTH_ATTACHMENT
            && attachment != STENCIL_ATTACHMENT && attachment != DEPTH_STENCIL_ATTACHMENT) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << "(): INVALID_OPERATION attachment must be one of "
                                     << "COLOR_ATTACHMENT0, DEPTH_ATTACHMENT, STENCIL_ATTACHMENT "
                                     << "or DEPTH_STENCIL_ATTACHMENT";
        m_error = INVALID_OPERATION;
        return;
    }

    if (renderbuffer && renderbuffertarget != RENDERBUFFER) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << "(): INVALID_OPERATION renderbuffertarget must be "
                                     << "RENDERBUFFER for non null renderbuffers";
        m_error = INVALID_OPERATION;
        return;
    }

    GLuint renderbufferId = renderbuffer ? renderbuffer->id() : 0;

    glFramebufferRenderbuffer(GLenum(target), GLenum(attachment), GLenum(renderbuffertarget),
                              renderbufferId);
}

/*!
 * \qmlmethod void Context3D::framebufferTexture2D(glEnums target, glEnums attachment, glEnums textarget, Texture3D texture, int level)
 * Attaches the given \a renderbuffer object to the \a attachment point of the current framebuffer
 * object.
 * \a target must be \c{Context3D.FRAMEBUFFER}. \a renderbuffertarget must
 * be \c{Context3D.RENDERBUFFER}.
 */
/*!
 * \internal
 */
void CanvasContext::framebufferTexture2D(glEnums target, glEnums attachment, glEnums textarget,
                                         CanvasTexture *texture, int level)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(target:" << glEnumToString(target)
                                << ", attachment:" << glEnumToString(attachment)
                                << ", textarget:" << glEnumToString(textarget)
                                << ", texture:" << texture
                                << ", level:" << level
                                << ")";

    if (target != FRAMEBUFFER) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << "(): INVALID_ENUM bind target, must be FRAMEBUFFER";
        m_error = INVALID_ENUM;
        return;
    }

    if (!m_currentFramebuffer) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << "(): INVALID_OPERATION no current framebuffer bound";
        m_error = INVALID_OPERATION;
        return;
    }

    if (attachment != COLOR_ATTACHMENT0 && attachment != DEPTH_ATTACHMENT
            && attachment != STENCIL_ATTACHMENT) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << "(): INVALID_OPERATION attachment must be one of "
                                     << "COLOR_ATTACHMENT0, DEPTH_ATTACHMENT or STENCIL_ATTACHMENT";
        m_error = INVALID_OPERATION;
        return;
    }

    if (texture) {
        // TODO: If texture is not zero, then texture must either name an existing texture
        //object with an target of textarget, or texture must name an existing cube map
        //texture and textarget must be one of:
        //TEXTURE_CUBE_MAP_POSITIVE_X, TEXTURE_CUBE_MAP_- POSITIVE_Y, TEXTURE_CUBE_MAP_POSITIVE_- Z,
        //TEXTURE_CUBE_MAP_NEGATIVE_X, TEXTURE_CUBE_MAP_NEGATIVE_Y, or TEXTURE_CUBE_MAP_NEGATIVE_Z.
        //Otherwise, INVALID_OPERATION is gener- ated.

        if (textarget != TEXTURE_2D
                && textarget != TEXTURE_CUBE_MAP_POSITIVE_X
                && textarget != TEXTURE_CUBE_MAP_POSITIVE_Y
                && textarget != TEXTURE_CUBE_MAP_POSITIVE_Z
                && textarget != TEXTURE_CUBE_MAP_NEGATIVE_X
                && textarget != TEXTURE_CUBE_MAP_NEGATIVE_Y
                && textarget != TEXTURE_CUBE_MAP_NEGATIVE_Z) {
            if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                         << "(): textarget attachment must be one of TEXTURE_2D, "
                                         << "TEXTURE_CUBE_MAP_POSITIVE_X, "
                                         << "TEXTURE_CUBE_MAP_POSITIVE_Y, "
                                         << "TEXTURE_CUBE_MAP_POSITIVE_Z, "
                                         << "TEXTURE_CUBE_MAP_NEGATIVE_X, "
                                         << "TEXTURE_CUBE_MAP_NEGATIVE_Y or "
                                         << "TEXTURE_CUBE_MAP_NEGATIVE_Z";
            m_error = INVALID_OPERATION;
            return;
        }

        if (level != 0) {
            if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                         << "(): INVALID_VALUE level must be 0";
            m_error = INVALID_VALUE;
            return;
        }
    }

    GLuint textureId = texture ? texture->textureId() : 0;
    glFramebufferTexture2D(GLenum(target), GLenum(attachment), GLenum(textarget), textureId, level);
}

/*!
 * \qmlmethod void Context3D::isFramebuffer(Object anyObject)
 * Returns true if the given object is a valid FrameBuffer3D object.
 * \a anyObject is the object that is to be verified as a valid framebuffer.
 */
/*!
 * \internal
 */
bool CanvasContext::isFramebuffer(QObject *anyObject)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "( anyObject:" << anyObject
                                << ")";

    if (!anyObject)
        return false;

    QString className = QString(anyObject->metaObject()->className());
    if (className != "CanvasFrameBuffer")
        return false;

    CanvasFrameBuffer *fbo = static_cast<CanvasFrameBuffer *>(anyObject);
    if (!fbo->isAlive())
        return false;

    return glIsFramebuffer(fbo->id());
}

/*!
 * \qmlmethod void Context3D::deleteFramebuffer(FrameBuffer3D buffer)
 * Deletes the given framebuffer as if by calling \c{glDeleteFramebuffers()}.
 * Calling this method repeatedly on the same object has no side effects.
 * \a buffer is the FrameBuffer3D to be deleted.
 */
/*!
 * \internal
 */
void CanvasContext::deleteFramebuffer(CanvasFrameBuffer *buffer)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "( buffer:" << buffer
                                << ")";

    if (buffer) {
        buffer->del();
    } else {
        m_error = INVALID_VALUE;
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << "(): INVALID_VALUE buffer handle";
    }
}

/*!
 * \qmlmethod RenderBuffer3D Context3D::createRenderbuffer()
 * Returns a created RenderBuffer3D object that is initialized with a renderbuffer object name
 * as if by calling \c{glGenRenderbuffers()}.
 */
/*!
 * \internal
 */
CanvasRenderBuffer *CanvasContext::createRenderbuffer()
{
    CanvasRenderBuffer *renderbuffer = new CanvasRenderBuffer(this);
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "():" << renderbuffer;

    // Returning a pointer to QObject that has parent set
    // -> V4VM should respect this and ownership should remain with this class
    return renderbuffer;
}

/*!
 * \qmlmethod void Context3D::bindRenderbuffer(glEnums target, RenderBuffer3D renderbuffer)
 * Binds the given \a renderbuffer object to the given \a target.
 * \a target must be \c{Context3D.RENDERBUFFER}.
 */
/*!
 * \internal
 */
void CanvasContext::bindRenderbuffer(glEnums target, CanvasRenderBuffer *renderbuffer)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(target:" << glEnumToString(target)
                                << ", renderbuffer:" << renderbuffer
                                << ")";

    if (target != RENDERBUFFER) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ": INVALID_ENUM target must be RENDERBUFFER";
        m_error = INVALID_ENUM;
        return;
    }

    if (renderbuffer) {
        m_currentRenderbuffer = renderbuffer;
        glBindRenderbuffer(GL_FRAMEBUFFER, renderbuffer->id());
    } else {
        m_currentRenderbuffer = 0;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

/*!
 * \qmlmethod void Context3D::renderbufferStorage(glEnums target, glEnums internalformat, int width, int height)
 * Create and initialize a data store for the \c renderbuffer object.
 * \a target must be \c Context3D.RENDERBUFFER.
 * \a internalformat specifies the color-renderable, depth-renderable or stencil-renderable format
 * of the renderbuffer. Must be one of \c{Context3D.RGBA4}, \c{Context3D.RGB565}, \c{Context3D.RGB5_A1},
 * \c{Context3D.DEPTH_COMPONENT16} or \c{Context3D.STENCIL_INDEX8}.
 * \a width specifies the renderbuffer width in pixels.
 * \a height specifies the renderbuffer height in pixels.
 */
/*!
 * \internal
 */
void CanvasContext::renderbufferStorage(glEnums target, glEnums internalformat,
                                        int width, int height)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(target:" << glEnumToString(target)
                                << ", internalformat:" << glEnumToString(internalformat)
                                << ", width:" << width
                                << ", height:" << height
                                << ")";

    if (target != RENDERBUFFER) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ": INVALID_ENUM target must be RENDERBUFFER";
        m_error = INVALID_ENUM;
        return;
    }

    glRenderbufferStorage(GLenum(target), GLenum(internalformat), width, height);
}

/*!
 * \qmlmethod bool Context3D::isRenderbuffer(Object anyObject)
 * Returns true if the given object is a valid RenderBuffer3D object.
 * \a anyObject is the object that is to be verified as a valid renderbuffer.
 */
/*!
 * \internal
 */
bool CanvasContext::isRenderbuffer(QObject *anyObject)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(anyObject:" << anyObject
                                << ")";

    if (!anyObject)
        return false;

    QString className = QString(anyObject->metaObject()->className());
    if (className != "CanvasRenderBuffer")
        return false;

    CanvasRenderBuffer *rbo = static_cast<CanvasRenderBuffer *>(anyObject);
    if (!rbo->isAlive())
        return false;
    return glIsRenderbuffer(rbo->id());
}

/*!
 * \qmlmethod void Context3D::deleteRenderbuffer(RenderBuffer3D buffer)
 * Deletes the given renderbuffer as if by calling \c{glDeleteRenderbuffers()}.
 * Calling this method repeatedly on the same object has no side effects.
 * \a buffer is the RenderBuffer3D to be deleted.
 */
/*!
 * \internal
 */
void CanvasContext::deleteRenderbuffer(CanvasRenderBuffer *renderbuffer)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(renderbuffer:" << renderbuffer
                                << ")";

    if (renderbuffer) {
        renderbuffer->del();
    } else {
        m_error = INVALID_VALUE;
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << "(): INVALID_VALUE renderbuffer handle";
    }
}

/*!
 * \qmlmethod void Context3D::sampleCoverage(float value, bool invert)
 * Sets the multisample coverage parameters.
 * \a value specifies the floating-point sample coverage value. The value is clamped to the range
 * \c{[0, 1]} with initial value of \c{1.0}.
 * \a invert specifies if coverage masks should be inverted.
 */
/*!
 * \internal
 */
void CanvasContext::sampleCoverage(float value, bool invert)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(value:" << value
                                << ", invert:" << invert
                                << ")";
    glSampleCoverage(value, invert);
}

/*!
 * \qmlmethod Program3D Context3D::createProgram()
 * Returns a created Program3D object that is initialized with a program object name as if by
 * calling \c{glCreateProgram()}.
 */
/*!
 * \internal
 */
CanvasProgram *CanvasContext::createProgram()
{
    CanvasProgram *program = new CanvasProgram(this);
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "():" << program;

    // Returning a pointer to QObject that has parent set
    // -> V4VM should respect this and ownership should remain with this class
    return program;
}

/*!
 * \qmlmethod bool Context3D::isProgram(Object anyObject)
 * Returns true if the given object is a valid Program3D object.
 * \a anyObject is the object that is to be verified as a valid program.
 */
/*!
 * \internal
 */
bool CanvasContext::isProgram(QObject *anyObject)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(anyObject:" << anyObject
                                << ")";

    if (!anyObject)
        return false;

    QString className = QString(anyObject->metaObject()->className());
    if (className != "CanvasProgram")
        return false;

    CanvasProgram *program = static_cast<CanvasProgram *>(anyObject);
    return program->isAlive();
}

/*!
 * \qmlmethod void Context3D::deleteProgram(Program3D program)
 * Deletes the given program as if by calling \c{glDeleteProgram()}.
 * Calling this method repeatedly on the same object has no side effects.
 * \a program is the Program3D to be deleted.
 */
/*!
 * \internal
 */
void CanvasContext::deleteProgram(CanvasProgram *program)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(program:" << program
                                << ")";

    if (program) {
        program->del();
    } else {
        m_error = INVALID_VALUE;
        if (m_logAllErrors) qDebug() << "deleteProgram(): INVALID_VALUE program handle:" << program;
    }
}

/*!
 * \qmlmethod void Context3D::attachShader(Program3D program, Shader3D shader)
 * Attaches the given \a shader object to the given \a program object.
 * Calling this method repeatedly on the same object has no side effects.
 * \a program is the Program3D to be deleted.
 */
/*!
 * \internal
 */
void CanvasContext::attachShader(CanvasProgram *program, CanvasShader *shader)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(program:" << program
                                << ", shader:" << shader
                                << ")";
    if (!program || !shader || !program->isAlive())
        return;

    program->attach(shader);
}

/*!
 * \qmlmethod list<Shader3D> Context3D::getAttachedShaders(Program3D program)
 * Returns the list of shaders currently attached to the given \a program.
 */
/*!
 * \internal
 */
QVariantList CanvasContext::getAttachedShaders(CanvasProgram *program)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(program:" << program
                                << ")";

    QVariantList shaderList;
    if (!program)
        return shaderList;

    QList<CanvasShader *> shaders = program->attachedShaders();

    for (QList<CanvasShader *>::const_iterator iter = shaders.constBegin();
         iter != shaders.constEnd(); iter++) {
        CanvasShader *shader = *iter;
        shaderList << QVariant::fromValue(shader);
    }

    return shaderList;
}


/*!
 * \qmlmethod void Context3D::detachShader(Program3D program, Shader3D shader)
 * Detaches given shader object from given program object.
 * \a program specifies the program object from which to detach the shader.
 * \a shader specifies the shader object to detach.
 */
/*!
 * \internal
 */
void CanvasContext::detachShader(CanvasProgram *program, CanvasShader *shader)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(program:" << program
                                << ", shader:" << shader
                                << ")";
    if (!program || !shader || !program->isAlive())
        return;

    program->detach(shader);
}

/*!
 * \qmlmethod void Context3D::linkProgram(Program3D program)
 * Links the given program object.
 * \a program specifies the program to be linked.
 */
/*!
 * \internal
 */
void CanvasContext::linkProgram(CanvasProgram *program)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(program:" << program
                                << ")";
    if (!program || !program->isAlive())
        return;

    program->link();
}

/*!
 * \qmlmethod void Context3D::lineWidth(float width)
 * Specifies the width of rasterized lines.
 * \a width specifies the width to be used when rasterizing lines. Initial value is \c{1.0}.
 */
/*!
 * \internal
 */
void CanvasContext::lineWidth(float width)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(width:" << width
                                << ")";
    glLineWidth(width);
}

/*!
 * \qmlmethod void Context3D::polygonOffset(float factor, float units)
 * Sets scale and units used to calculate depth values.
 * \a factor specifies the scale factor that is used to create a variable depth offset for each
 * polygon. Initial value is \c{0.0}.
 * \a units gets multiplied by an implementation-specific value to create a constant depth offset.
 * Initial value is \c{0.0}.
 */
/*!
 * \internal
 */
void CanvasContext::polygonOffset(float factor, float units)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(factor:" << factor
                                << ", units:" << units
                                << ")";
    glPolygonOffset(factor, units);
}

/*!
 * \qmlmethod void Context3D::pixelStorei(glEnums pname, int param)
 * Set the pixel storage modes.
 * \a pname specifies the name of the parameter to be set. \c Context3D.PACK_ALIGNMENT affects the
 * packing of pixel data into memory. \c Context3D.UNPACK_ALIGNMENT affects the unpacking of pixel
 * data from memory.
 * \a param specifies the value that \a pname is set to.
 */
/*!
 * \internal
 */
void CanvasContext::pixelStorei(glEnums pname, int param)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(pname:" << glEnumToString(pname)
                                << ", param:" << param
                                << ")";

    switch (pname) {
    case UNPACK_FLIP_Y_WEBGL:
        m_unpackFlipYEnabled = (param != 0);
        break;
    case UNPACK_PREMULTIPLY_ALPHA_WEBGL:
        m_unpackPremultiplyAlphaEnabled = (param != 0);
        break;
    case UNPACK_COLORSPACE_CONVERSION_WEBGL:
        m_unpackColorspaceConversion = glEnums(param);
        break;
    default:
        glPixelStorei(GLenum(pname), param);
        break;
    }
}

/*!
 * \qmlmethod void Context3D::hint(glEnums target, glEnums mode)
 * Set implementation-specific hints.
 * \a target \c Context3D.GENERATE_MIPMAP_HINT is accepted.
 * \a mode \c{Context3D.FASTEST}, \c{Context3D.NICEST}, and \c{Context3D.DONT_CARE} are accepted.
 */
/*!
 * \internal
 */
void CanvasContext::hint(glEnums target, glEnums mode)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(target:" << glEnumToString(target)
                                << ",mode:" << glEnumToString(mode) << ")";
    glHint(GLenum(target), GLenum(mode));
}

/*!
 * \qmlmethod void Context3D::enable(glEnums cap)
 * Enable server side GL capabilities.
 * \a cap specifies a constant indicating a GL capability.
 */
/*!
 * \internal
 */
void CanvasContext::enable(glEnums cap)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(cap:" << glEnumToString(cap)
                                << ")";
    glEnable(cap);
}

/*!
 * \qmlmethod bool Context3D::isEnabled(glEnums cap)
 * Returns whether a capability is enabled.
 * \a cap specifies a constant indicating a GL capability.
 */
/*!
 * \internal
 */
bool CanvasContext::isEnabled(glEnums cap)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(cap:" << glEnumToString(cap)
                                << ")";
    return glIsEnabled(cap);
}

/*!
 * \qmlmethod void Context3D::disable(glEnums cap)
 * Disable server side GL capabilities.
 * \a cap specifies a constant indicating a GL capability.
 */
/*!
 * \internal
 */
void CanvasContext::disable(glEnums cap)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(cap:" << glEnumToString(cap)
                                << ")";
    glDisable(cap);
}

/*!
 * \qmlmethod void Context3D::blendColor(float red, float green, float blue, float alpha)
 * Set the blend color.
 * \a red, \a green, \a blue and \a alpha specify the components of \c{Context3D.BLEND_COLOR}.
 */
/*!
 * \internal
 */
void CanvasContext::blendColor(float red, float green, float blue, float alpha)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                <<  "(red:" << red
                                 << ", green:" << green
                                 << ", blue:" << blue
                                 << ", alpha:" << alpha
                                 << ")";
    glBlendColor(red, green, blue, alpha);
}

/*!
 * \qmlmethod void Context3D::blendEquation(glEnums mode)
 * Sets the equation used for both the RGB blend equation and the alpha blend equation
 * \a mode specifies how source and destination colors are to be combined. Must be
 * \c{Context3D.FUNC_ADD}, \c{Context3D.FUNC_SUBTRACT} or \c{Context3D.FUNC_REVERSE_SUBTRACT}.
 */
/*!
 * \internal
 */
void CanvasContext::blendEquation(glEnums mode)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(mode:" << glEnumToString(mode)
                                << ")";
    glBlendEquation(GLenum(mode));
}

/*!
 * \qmlmethod void Context3D::blendEquationSeparate(glEnums modeRGB, glEnums modeAlpha)
 * Set the RGB blend equation and the alpha blend equation separately.
 * \a modeRGB specifies how the RGB components of the source and destination colors are to be
 * combined. Must be \c{Context3D.FUNC_ADD}, \c{Context3D.FUNC_SUBTRACT} or
 * \c{Context3D.FUNC_REVERSE_SUBTRACT}.
 * \a modeAlpha specifies how the alpha component of the source and destination colors are to be
 * combined. Must be \c{Context3D.FUNC_ADD}, \c{Context3D.FUNC_SUBTRACT}, or
 * \c{Context3D.FUNC_REVERSE_SUBTRACT}.
 */
/*!
 * \internal
 */
void CanvasContext::blendEquationSeparate(glEnums modeRGB, glEnums modeAlpha)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(modeRGB:" << glEnumToString(modeRGB)
                                << ", modeAlpha:" << glEnumToString(modeAlpha)
                                << ")";
    glBlendEquationSeparate(GLenum(modeRGB), GLenum(modeAlpha));
}

/*!
 * \qmlmethod void Context3D::blendFunc(glEnums sfactor, glEnums dfactor)
 * Sets the pixel arithmetic.
 * \a sfactor specifies how the RGBA source blending factors are computed. Must be
 * \c{Context3D.ZERO}, \c{Context3D.ONE}, \c{Context3D.SRC_COLOR},
 * \c{Context3D.ONE_MINUS_SRC_COLOR},
 * \c{Context3D.DST_COLOR}, \c{Context3D.ONE_MINUS_DST_COLOR}, \c{Context3D.SRC_ALPHA},
 * \c{Context3D.ONE_MINUS_SRC_ALPHA}, \c{Context3D.DST_ALPHA}, \c{Context3D.ONE_MINUS_DST_ALPHA},
 * \c{Context3D.CONSTANT_COLOR}, \c{Context3D.ONE_MINUS_CONSTANT_COLOR},
 * \c{Context3D.CONSTANT_ALPHA},
 * \c{Context3D.ONE_MINUS_CONSTANT_ALPHA} or \c{Context3D.SRC_ALPHA_SATURATE}. Initial value is
 * \c{Context3D.ONE}.
 * \a dfactor Specifies how the RGBA destination blending factors are computed. Must be
 * \c{Context3D.ZERO}, \c{Context3D.ONE}, \c{Context3D.SRC_COLOR},
 * \c{Context3D.ONE_MINUS_SRC_COLOR},
 * \c{Context3D.DST_COLOR}, \c{Context3D.ONE_MINUS_DST_COLOR}, \c{Context3D.SRC_ALPHA},
 * \c{Context3D.ONE_MINUS_SRC_ALPHA}, \c{Context3D.DST_ALPHA}, \c{Context3D.ONE_MINUS_DST_ALPHA},
 * \c{Context3D.CONSTANT_COLOR}, \c{Context3D.ONE_MINUS_CONSTANT_COLOR},
 * \c{Context3D.CONSTANT_ALPHA} or
 * \c{Context3D.ONE_MINUS_CONSTANT_ALPHA}. Initial value is \c{Context3D.ZERO}.
 */
/*!
 * \internal
 */
void CanvasContext::blendFunc(glEnums sfactor, glEnums dfactor)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(sfactor:" << glEnumToString(sfactor)
                                << ", dfactor:" << glEnumToString(dfactor)
                                << ")";

    if (((sfactor == CONSTANT_COLOR || sfactor == ONE_MINUS_CONSTANT_COLOR)
         && (dfactor == CONSTANT_ALPHA || dfactor == ONE_MINUS_CONSTANT_ALPHA))
            || ((dfactor == CONSTANT_COLOR || dfactor == ONE_MINUS_CONSTANT_COLOR)
                && (sfactor == CONSTANT_ALPHA || sfactor == ONE_MINUS_CONSTANT_ALPHA))) {
        m_error = INVALID_OPERATION;
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ": INVALID_OPERATION illegal combination";
        return;
    }

    glBlendFunc(GLenum(sfactor), GLenum(dfactor));
}

/*!
 * \qmlmethod void Context3D::blendFuncSeparate(glEnums srcRGB, glEnums dstRGB, glEnums srcAlpha, glEnums dstAlpha)
 * Sets the pixel arithmetic for RGB and alpha components separately.
 * \a srcRGB specifies how the RGB source blending factors are computed. Must be \c{Context3D.ZERO},
 * \c{Context3D.ONE}, \c{Context3D.SRC_COLOR}, \c{Context3D.ONE_MINUS_SRC_COLOR},
 * \c{Context3D.DST_COLOR},
 * \c{Context3D.ONE_MINUS_DST_COLOR}, \c{Context3D.SRC_ALPHA}, \c{Context3D.ONE_MINUS_SRC_ALPHA},
 * \c{Context3D.DST_ALPHA}, \c{Context3D.ONE_MINUS_DST_ALPHA}, \c{Context3D.CONSTANT_COLOR},
 * \c{Context3D.ONE_MINUS_CONSTANT_COLOR}, \c{Context3D.CONSTANT_ALPHA},
 * \c{Context3D.ONE_MINUS_CONSTANT_ALPHA} or \c{Context3D.SRC_ALPHA_SATURATE}. Initial value is
 * \c{Context3D.ONE}.
 * \a dstRGB Specifies how the RGB destination blending factors are computed. Must be
 * \c{Context3D.ZERO}, \c{Context3D.ONE}, \c{Context3D.SRC_COLOR}, \c{Context3D.ONE_MINUS_SRC_COLOR},
 * \c{Context3D.DST_COLOR}, \c{Context3D.ONE_MINUS_DST_COLOR}, \c{Context3D.SRC_ALPHA},
 * \c{Context3D.ONE_MINUS_SRC_ALPHA}, \c{Context3D.DST_ALPHA}, \c{Context3D.ONE_MINUS_DST_ALPHA},
 * \c{Context3D.CONSTANT_COLOR}, \c{Context3D.ONE_MINUS_CONSTANT_COLOR},
 * \c{Context3D.CONSTANT_ALPHA} or
 * \c{Context3D.ONE_MINUS_CONSTANT_ALPHA}. Initial value is \c{Context3D.ZERO}.
 * \a srcAlpha specifies how the alpha source blending factors are computed. Must be
 * \c{Context3D.ZERO}, \c{Context3D.ONE}, \c{Context3D.SRC_COLOR},
 * \c{Context3D.ONE_MINUS_SRC_COLOR},
 * \c{Context3D.DST_COLOR}, \c{Context3D.ONE_MINUS_DST_COLOR}, \c{Context3D.SRC_ALPHA},
 * \c{Context3D.ONE_MINUS_SRC_ALPHA}, \c{Context3D.DST_ALPHA}, \c{Context3D.ONE_MINUS_DST_ALPHA},
 * \c{Context3D.CONSTANT_COLOR}, \c{Context3D.ONE_MINUS_CONSTANT_COLOR},
 * \c{Context3D.CONSTANT_ALPHA},
 * \c{Context3D.ONE_MINUS_CONSTANT_ALPHA} or \c{Context3D.SRC_ALPHA_SATURATE}. Initial value is
 * \c{Context3D.ONE}.
 * \a dstAlpha Specifies how the alpha destination blending factors are computed. Must be
 * \c{Context3D.ZERO}, \c{Context3D.ONE}, \c{Context3D.SRC_COLOR},
 * \c{Context3D.ONE_MINUS_SRC_COLOR},
 * \c{Context3D.DST_COLOR}, \c{Context3D.ONE_MINUS_DST_COLOR}, \c{Context3D.SRC_ALPHA},
 * \c{Context3D.ONE_MINUS_SRC_ALPHA}, \c{Context3D.DST_ALPHA}, \c{Context3D.ONE_MINUS_DST_ALPHA},
 * \c{Context3D.CONSTANT_COLOR}, \c{Context3D.ONE_MINUS_CONSTANT_COLOR},
 * \c{Context3D.CONSTANT_ALPHA} or
 * \c{Context3D.ONE_MINUS_CONSTANT_ALPHA}. Initial value is \c{Context3D.ZERO}.
 */
/*!
 * \internal
 */
void CanvasContext::blendFuncSeparate(glEnums srcRGB, glEnums dstRGB, glEnums srcAlpha,
                                      glEnums dstAlpha)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(srcRGB:" << glEnumToString(srcRGB)
                                << ", dstRGB:" << glEnumToString(dstRGB)
                                << ", srcAlpha:" << glEnumToString(srcAlpha)
                                << ", dstAlpha:" << glEnumToString(dstAlpha)
                                << ")";

    if (((srcRGB == CONSTANT_COLOR || srcRGB == ONE_MINUS_CONSTANT_COLOR )
         && (dstRGB == CONSTANT_ALPHA || dstRGB == ONE_MINUS_CONSTANT_ALPHA ))
            || ((dstRGB == CONSTANT_COLOR || dstRGB == ONE_MINUS_CONSTANT_COLOR )
                && (srcRGB == CONSTANT_ALPHA || srcRGB == ONE_MINUS_CONSTANT_ALPHA ))) {
        m_error = INVALID_OPERATION;
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ": INVALID_OPERATION illegal combination";
        return;
    }

    glBlendFuncSeparate(GLenum(srcRGB), GLenum(dstRGB), GLenum(srcAlpha), GLenum(dstAlpha));
}

/*!
 * \qmlmethod variant Context3D::getProgramParameter(Program3D program, glEnums paramName)
 * Return the value for the passed \a paramName given the passed \a program. The type returned is
 * the natural type for the requested paramName.
 * \a paramName must be \c{Context3D.DELETE_STATUS}, \c{Context3D.LINK_STATUS},
 * \c{Context3D.VALIDATE_STATUS}, \c{Context3D.ATTACHED_SHADERS}, \c{Context3D.ACTIVE_ATTRIBUTES} or
 * \c{Context3D.ACTIVE_UNIFORMS}.
 */
/*!
 * \internal
 */
QVariant CanvasContext::getProgramParameter(CanvasProgram *program, glEnums paramName)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(program:" << program
                                << ", paramName:" << glEnumToString(paramName)
                                << ")";

    if (!program || !program->isAlive())
        return 0;

    switch(paramName) {
    case DELETE_STATUS:
        // Intentional flow through
    case LINK_STATUS:
        // Intentional flow through
    case VALIDATE_STATUS: {
        GLint value = 0;
        glGetProgramiv(program->id(), GLenum(paramName), &value);
        if (m_logAllCalls) qDebug() << "    getProgramParameter returns " << value;
        return QVariant::fromValue(value == GL_TRUE);
    }
    case ATTACHED_SHADERS:
        // Intentional flow through
    case ACTIVE_ATTRIBUTES:
        // Intentional flow through
    case ACTIVE_UNIFORMS: {
        GLint value = 0;
        glGetProgramiv(program->id(), GLenum(paramName), &value);
        if (m_logAllCalls) qDebug() << "    getProgramParameter returns " << value;
        return QVariant::fromValue(value);
    }
    default: {
        // TODO: Implement rest of the parameters
        m_error = INVALID_ENUM;
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ": INVALID_ENUM illegal parameter name ";
        return QVariant::fromValue(0);
    }
    }
}

/*!
 * \qmlmethod Shader3D Context3D::createShader(glEnums type)
 * Creates a shader of \a type. Must be either \c Context3D.VERTEX_SHADER or
 * \c{Context3D.FRAGMENT_SHADER}.
 */
/*!
 * \internal
 */
CanvasShader *CanvasContext::createShader(glEnums type)
{
    switch (type) {
    case VERTEX_SHADER:
        if (m_logAllCalls) qDebug() << "Context3D::createShader(VERTEX_SHADER)";
        // Returning a pointer to QObject that has parent set
        // -> V4VM should respect this and ownership should remain with this class
        return new CanvasShader(QOpenGLShader::Vertex, this);
    case FRAGMENT_SHADER:
        if (m_logAllCalls) qDebug() << "Context3D::createShader(FRAGMENT_SHADER)";
        // Returning a pointer to QObject that has parent set
        // -> V4VM should respect this and ownership should remain with this class
        return new CanvasShader(QOpenGLShader::Fragment, this);
    default:
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_ENUM unknown shader type:"
                                     << glEnumToString(type);
        m_error = INVALID_ENUM;
        return 0;
    }
}

/*!
 * \qmlmethod bool Context3D::isShader(Object anyObject)
 * Returns true if the given object is a valid Shader3D object.
 * \a anyObject is the object that is to be verified as a valid shader.
 */
/*!
 * \internal
 */
bool CanvasContext::isShader(QObject *anyObject)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(anyObject:" << anyObject
                                << ")";

    if (!anyObject)
        return false;

    QString className = QString(anyObject->metaObject()->className());
    if (className != "CanvasShader")
        return false;

    CanvasShader *shader = static_cast<CanvasShader *>(anyObject);
    return shader->isAlive();
}

/*!
 * \qmlmethod void Context3D::deleteShader(Shader3D shader)
 * Deletes the given shader as if by calling \c{glDeleteShader()}.
 * Calling this method repeatedly on the same object has no side effects.
 * \a shader is the Shader3D to be deleted.
 */
/*!
 * \internal
 */
void CanvasContext::deleteShader(CanvasShader *shader)
{
    if (m_logAllCalls) qDebug() << "Context3D::"
                                << __FUNCTION__
                                << "(shader:" << shader
                                << ")";

    if (shader) {
        shader->del();
    } else {
        m_error = INVALID_VALUE;
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ": invalid shader handle:" << shader;
    }
}

/*!
 * \qmlmethod void Context3D::shaderSource(Shader3D shader, string shaderSource)
 * Replaces the shader source code in the given shader object.
 * \a shader specifies the shader object whose source code is to be replaced.
 * \a shaderSource specifies the source code to be loaded in to the shader.
 */
/*!
 * \internal
 */
void CanvasContext::shaderSource(CanvasShader *shader, const QString &shaderSource)
{
    QString modSource = "#version 120 \n#define precision \n"+ shaderSource;

    if (m_isOpenGLES2)
        modSource = shaderSource;

    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(shader:" << shader
                                << ", shaderSource"
                                << ")" << endl << modSource << endl;
    if (!shader) {
        m_error = INVALID_VALUE;
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ": invalid shader handle:" << shader;
        return;
    }
    shader->setSourceCode(modSource);
}


/*!
 * \qmlmethod string Context3D::getShaderSource(Shader3D shader)
 * Returns the source code string from the \a shader object.
 */
/*!
 * \internal
 */
QString CanvasContext::getShaderSource(CanvasShader *shader)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(shader:" << shader
                                << ")";
    if (!shader) {
        m_error = INVALID_VALUE;
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     <<": invalid shader handle:" << shader;
        return m_emptyString;
    }

    // Returning a copy, V4VM takes ownership
    return QString(shader->qOGLShader()->sourceCode());
}

/*!
 * \qmlmethod void Context3D::compileShader(Shader3D shader)
 * Compiles the given \a shader object.
 */
/*!
 * \internal
 */
void CanvasContext::compileShader(CanvasShader *shader)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(shader:" << shader
                                << ")";
    if (!shader) {
        m_error = INVALID_VALUE;
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ": invalid shader handle:" << shader;
        return;
    }
    shader->qOGLShader()->compileSourceCode(shader->sourceCode());
}

/*!
 * \qmlmethod void Context3D::uniform1i(UniformLocation location, int x)
 * Sets the single integer value given in \a x to the given uniform \a location.
 */
/*!
 * \internal
 */
void CanvasContext::uniform1i(CanvasUniformLocation *location, int x)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(location:" << location
                                << ", x:" << x
                                << ")";
    if (!location)
        return;

    glUniform1i(location->id(), x);
}

/*!
 * \qmlmethod void Context3D::uniform1iv(UniformLocation location, Int32Array array)
 * Sets the integer array given in \a array to the given uniform \a location.
 */
/*!
 * \internal
 */
void CanvasContext::uniform1iv(CanvasUniformLocation *location, CanvasInt32Array *array)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(location:" << location
                                << ", array:" << array
                                << ")";
    if (!location || !array)
        return;

    glUniform1iv(location->id(), array->length(), (int *)array->rawDataCptr());
}

/*!
 * \qmlmethod void Context3D::uniform1f(UniformLocation location, float x)
 * Sets the single float value given in \a x to the given uniform \a location.
 */
/*!
 * \internal
 */
void CanvasContext::uniform1f(CanvasUniformLocation *location, float x)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(location:" << location
                                << ", x:" << x
                                << ")";
    if (!location)
        return;

    glUniform1f(location->id(), x);
}

/*!
 * \qmlmethod void Context3D::uniform1fv(UniformLocation location, Float32Array array)
 * Sets the float array given in \a array to the given uniform \a location.
 */
/*!
 * \internal
 */
void CanvasContext::uniform1fv(CanvasUniformLocation *location, CanvasFloat32Array *array)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(location:" << location
                                << ", array:" << array
                                << ")";
    if (!location || !array)
        return;

    glUniform1fv(location->id(), array->length(), (float *)array->rawDataCptr());
}

/*!
 * \qmlmethod void Context3D::uniform2f(UniformLocation location, float x, float y)
 * Sets the two float values given in \a x and \a y to the given uniform \a location.
 */
/*!
 * \internal
 */
void CanvasContext::uniform2f(CanvasUniformLocation *location, float x, float y)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(location:" << location
                                << ", x:" << x
                                << ", y:" << y
                                << ")";
    if (!location)
        return;

    glUniform2f(location->id(), x, y);
}

/*!
 * \qmlmethod void Context3D::uniform2fv(UniformLocation location, Float32Array array)
 * Sets the float array given in \a array to the given uniform \a location.
 */
/*!
 * \internal
 */
void CanvasContext::uniform2fv(CanvasUniformLocation *location, CanvasFloat32Array *array)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(location:" << location
                                << ", array:" << array
                                << ")";
    if (!location || !array)
        return;

    glUniform2fv(location->id(), array->length() / 2, (float *)array->rawDataCptr());
}

/*!
 * \qmlmethod void Context3D::uniform2i(UniformLocation location, int x, int y)
 * Sets the two integer values given in \a x and \a y to the given uniform \a location.
 */
/*!
 * \internal
 */
void CanvasContext::uniform2i(CanvasUniformLocation *location, int x, int y)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(location:" << location
                                << ", x:" << x
                                << ", y:" << y
                                << ")";
    if (!location)
        return;

    glUniform2i(location->id(), x, y);

}

/*!
 * \qmlmethod void Context3D::uniform2iv(UniformLocation location, Int32Array array)
 * Sets the integer array given in \a array to the given uniform \a location.
 */
/*!
 * \internal
 */
void CanvasContext::uniform2iv(CanvasUniformLocation *location, CanvasInt32Array *array)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(location:" << location
                                << ", array:" << array
                                << ")";
    if (!location || !array)
        return;

    glUniform2iv(location->id(), array->length() / 2, (int *)array->rawDataCptr());
}

/*!
 * \qmlmethod void Context3D::uniform3f(UniformLocation location, float x, float y, float z)
 * Sets the three float values given in \a x , \a y and \a z to the given uniform \a location.
 */
/*!
 * \internal
 */
void CanvasContext::uniform3f(CanvasUniformLocation *location, float x, float y, float z)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(location:" << location
                                << ", x:" << x
                                << ", y:" << y
                                << ", z:" << z
                                << ")";
    if (!location)
        return;

    glUniform3f(location->id(), x, y, z);

}

/*!
 * \qmlmethod void Context3D::uniform3fv(UniformLocation location, Float32Array array)
 * Sets the float array given in \a array to the given uniform \a location.
 */
/*!
 * \internal
 */
void CanvasContext::uniform3fv(CanvasUniformLocation *location, CanvasFloat32Array *array)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(location:" << location
                                << ", array:" << array
                                << ")";
    if (!location || !array)
        return;

    glUniform3fv(location->id(), array->length() / 3, (float *)array->rawDataCptr());
}

/*!
 * \qmlmethod void Context3D::uniform3i(UniformLocation location, int x, int y, int z)
 * Sets the three integer values given in \a x , \a y and \a z to the given uniform \a location.
 */
/*!
 * \internal
 */
void CanvasContext::uniform3i(CanvasUniformLocation *location, int x, int y, int z)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(location:" << location
                                << ", x:" << x
                                << ", y:" << y
                                << ", z:" << z
                                << ")";
    if (!location)
        return;

    glUniform3i(location->id(), x, y, z);
}

/*!
 * \qmlmethod void Context3D::uniform3iv(UniformLocation location, Int32Array array)
 * Sets the integer array given in \a array to the given uniform \a location.
 */
/*!
 * \internal
 */
void CanvasContext::uniform3iv(CanvasUniformLocation *location, CanvasInt32Array *array)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(location:" << location
                                << ", array:" << array
                                << ")";
    if (!location || !array)
        return;

    glUniform3iv(location->id(), array->length() / 3, (int *)array->rawDataCptr());
}

/*!
 * \qmlmethod void Context3D::uniform4f(UniformLocation location, float x, float y, float z, float w)
 * Sets the four float values given in \a x , \a y , \a z and \a w to the given uniform \a location.
 */
/*!
 * \internal
 */
void CanvasContext::uniform4f(CanvasUniformLocation *location, float x, float y, float z, float w)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(location:" << location
                                << ", x:" << x
                                << ", y:" << y
                                << ", z:" << z
                                << ", w:" << w
                                << ")";
    if (!location)
        return;

    glUniform4f(location->id(), x, y, z, w);

}

/*!
 * \qmlmethod void Context3D::uniform4fv(UniformLocation location, Float32Array array)
 * Sets the float array given in \a array to the given uniform \a location.
 */
/*!
 * \internal
 */
void CanvasContext::uniform4fv(CanvasUniformLocation *location, CanvasFloat32Array *array)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(location:" << location
                                << ", array:" << array
                                << ")";
    if (!location || !array)
        return;

    glUniform4fv(location->id(), array->length() / 4, (float *)array->rawDataCptr());
}

/*!
 * \qmlmethod void Context3D::uniform4i(UniformLocation location, int x, int y, int z, int w)
 * Sets the four integer values given in \a x , \a y , \a z and \a w to the given uniform
 * \a location.
 */
/*!
 * \internal
 */
void CanvasContext::uniform4i(CanvasUniformLocation *location, int x, int y, int z, int w)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(location:" << location
                                << ", x:" << x
                                << ", y:" << y
                                << ", z:" << z
                                << ", w:" << w
                                << ")";
    if (!location)
        return;

    glUniform4i(location->id(), x, y, z, w);

}

/*!
 * \qmlmethod void Context3D::uniform4iv(UniformLocation location, Int32Array array)
 * Sets the integer array given in \a array to the given uniform \a location.
 */
/*!
 * \internal
 */
void CanvasContext::uniform4iv(CanvasUniformLocation *location, CanvasInt32Array *array)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(location:" << location
                                << ", array:" << array
                                << ")";
    if (!location || !array)
        return;

    glUniform4iv(location->id(), array->length() / 4, (int *)array->rawDataCptr());
}

/*!
 * \qmlmethod void Context3D::uniform1fva(UniformLocation location, list<variant> array)
 * Sets the float array given as JavasScript \a array to the given uniform \a location.
 */
/*!
 * \internal
 */
void CanvasContext::uniform1fva(CanvasUniformLocation *location, QVariantList array)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(location:" << location
                                << ", array:" << array
                                << ")";
    if (!location)
        return;

    float *arrayData = new float[array.length()];
    ArrayUtils::fillFloatArrayFromVariantList(array, arrayData);
    glUniform1fv(location->id(), array.count(), arrayData);
    delete [] arrayData;
}

/*!
 * \qmlmethod void Context3D::uniform2fva(UniformLocation location, list<variant> array)
 * Sets the float array given as JavasScript \a array to the given uniform \a location.
 */
/*!
 * \internal
 */
void CanvasContext::uniform2fva(CanvasUniformLocation *location, QVariantList array)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(location:" << location
                                << ", array:" << array
                                << ")";
    if (!location)
        return;

    float *arrayData = new float[array.length()];
    ArrayUtils::fillFloatArrayFromVariantList(array, arrayData);
    glUniform2fv(location->id(), array.count() / 2, arrayData);
    delete [] arrayData;
}

/*!
 * \qmlmethod void Context3D::uniform3fva(UniformLocation location, list<variant> array)
 * Sets the float array given as JavasScript \a array to the given uniform \a location.
 */
/*!
 * \internal
 */
void CanvasContext::uniform3fva(CanvasUniformLocation *location, QVariantList array)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(location:" << location
                                << ", array:" << array
                                << ")";
    if (!location)
        return;

    float *arrayData = new float[array.length()];
    ArrayUtils::fillFloatArrayFromVariantList(array, arrayData);
    glUniform3fv(location->id(), array.count() / 3, arrayData);
    delete [] arrayData;
}

/*!
 * \qmlmethod void Context3D::uniform4fva(UniformLocation location, list<variant> array)
 * Sets the float array given as JavasScript \a array to the given uniform \a location.
 */
/*!
 * \internal
 */
void CanvasContext::uniform4fva(CanvasUniformLocation *location, QVariantList array)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(location:" << location
                                << ", array:" << array
                                << ")";
    if (!location)
        return;

    float *arrayData = new float[array.count()];
    ArrayUtils::fillFloatArrayFromVariantList(array, arrayData);
    glUniform4fv(location->id(), array.count() / 4, arrayData);
    delete [] arrayData;
}

/*!
 * \qmlmethod void Context3D::uniform1iva(UniformLocation location, list<variant> array)
 * Sets the integer array given as JavasScript \a array to the given uniform \a location.
 */
/*!
 * \internal
 */
void CanvasContext::uniform1iva(CanvasUniformLocation *location, QVariantList array)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(location:" << location
                                << ", array:" << array
                                << ")";
    if (!location)
        return;

    int *arrayData = new int[array.length()];
    ArrayUtils::fillIntArrayFromVariantList(array, arrayData);
    glUniform1iv(location->id(), array.count(), arrayData);
    delete [] arrayData;

}

/*!
 * \qmlmethod void Context3D::uniform2iva(UniformLocation location, list<variant> array)
 * Sets the integer array given as JavasScript \a array to the given uniform \a location.
 */
/*!
 * \internal
 */
void CanvasContext::uniform2iva(CanvasUniformLocation *location, QVariantList array)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(location:" << location
                                << ", array:" << array
                                << ")";
    if (!location)
        return;

    int *arrayData = new int[array.length()];
    ArrayUtils::fillIntArrayFromVariantList(array, arrayData);
    glUniform2iv(location->id(), array.count() / 2, arrayData);
    delete [] arrayData;

}

/*!
 * \qmlmethod void Context3D::uniform3iva(UniformLocation location, list<variant> array)
 * Sets the integer array given as JavasScript \a array to the given uniform \a location.
 */
/*!
 * \internal
 */
void CanvasContext::uniform3iva(CanvasUniformLocation *location, QVariantList array)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(location:" << location
                                << ", array:" << array
                                << ")";
    if (!location)
        return;

    int *arrayData = new int[array.length()];
    ArrayUtils::fillIntArrayFromVariantList(array, arrayData);
    glUniform3iv(location->id(), array.count() / 3, arrayData);
    delete [] arrayData;

}

/*!
 * \qmlmethod void Context3D::uniform4iva(UniformLocation location, list<variant> array)
 * Sets the integer array given as JavasScript \a array to the given uniform \a location.
 */
/*!
 * \internal
 */
void CanvasContext::uniform4iva(CanvasUniformLocation *location, QVariantList array)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(location:" << location
                                << ", array:" << array
                                << ")";
    if (!location)
        return;

    int *arrayData = new int[array.length()];
    ArrayUtils::fillIntArrayFromVariantList(array, arrayData);
    glUniform4iv(location->id(), array.length() / 4, arrayData);
    delete [] arrayData;
}

/*!
 * \qmlmethod void Context3D::vertexAttrib1f(int indx, float x)
 * Sets the single float value given in \a x to the generic vertex attribute index specified
 * by \a indx.
 */
/*!
 * \internal
 */
void CanvasContext::vertexAttrib1f(unsigned int indx, float x)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(indx:" << indx
                                << ", x:" << x
                                << ")";
    glVertexAttrib1f(indx, x);
}

/*!
 * \qmlmethod void Context3D::vertexAttrib1fv(int indx, Float32Array array)
 * Sets the float array given in \a array to the generic vertex attribute index
 * specified by \a indx.
 */
/*!
 * \internal
 */
void CanvasContext::vertexAttrib1fv(unsigned int indx, CanvasFloat32Array *values)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(indx:" << indx
                                << ", values:" << values
                                << ")";
    glVertexAttrib1fv(indx, (float *)values->rawDataCptr());
}

/*!
 * \qmlmethod void Context3D::vertexAttrib2f(int indx, float x, float y)
 * Sets the two float values given in \a x and \a y to the generic vertex attribute index
 * specified by \a indx.
 */
/*!
 * \internal
 */
void CanvasContext::vertexAttrib2f(unsigned int indx, float x, float y)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(indx:" << indx
                                << ", x:" << x
                                << ", y:" << y
                                << ")";
    glVertexAttrib2f(indx, x, y);
}

/*!
 * \qmlmethod void Context3D::vertexAttrib2fv(int indx, Float32Array array)
 * Sets the float array given in \a array to the generic vertex attribute index
 * specified by \a indx.
 */
/*!
 * \internal
 */
void CanvasContext::vertexAttrib2fv(unsigned int indx, CanvasFloat32Array *values)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(indx:" << indx
                                << ", values:" << values
                                << ")";
    glVertexAttrib2fv(indx, (float *)values->rawDataCptr());
}

/*!
 * \qmlmethod void Context3D::vertexAttrib3f(int indx, float x, float y, float z)
 * Sets the three float values given in \a x , \a y and \a z to the generic vertex attribute index
 * specified by \a indx.
 */
/*!
 * \internal
 */
void CanvasContext::vertexAttrib3f(unsigned int indx, float x, float y, float z)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(indx:" << indx
                                << ", x:" << x
                                << ", y:" << y
                                << ", z:" << z
                                << ")";
    glVertexAttrib3f(indx, x, y, z);
}

/*!
 * \qmlmethod void Context3D::vertexAttrib3fv(int indx, Float32Array array)
 * Sets the float array given in \a array to the generic vertex attribute index
 * specified by \a indx.
 */
/*!
 * \internal
 */
void CanvasContext::vertexAttrib3fv(unsigned int indx, CanvasFloat32Array *values)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(indx:" << indx
                                << ", values:" << values
                                << ")";
    glVertexAttrib3fv(indx, (float *)values->rawDataCptr());
}

/*!
 * \qmlmethod void Context3D::vertexAttrib4f(int indx, float x, float y, float z, float w)
 * Sets the four float values given in \a x , \a y , \a z and \a w to the generic vertex attribute
 * index specified by \a indx.
 */
/*!
 * \internal
 */
void CanvasContext::vertexAttrib4f(unsigned int indx, float x, float y, float z, float w)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(indx:" << indx
                                << ", x:" << x
                                << ", y:" << y
                                << ", z:" << z
                                << ", w:" << w
                                << ")";
    glVertexAttrib4f(indx, x, y, z, w);
}

/*!
 * \qmlmethod void Context3D::vertexAttrib4fv(int indx, Float32Array array)
 * Sets the float array given in \a array to the generic vertex attribute index
 * specified by \a indx.
 */
/*!
 * \internal
 */
void CanvasContext::vertexAttrib4fv(unsigned int indx, CanvasFloat32Array *values)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(indx:" << indx
                                << ", values:" << values
                                << ")";
    glVertexAttrib4fv(indx, (float *)values->rawDataCptr());
}

/*!
 * \qmlmethod int Context3D::getShaderParameter(Shader3D shader, glEnums pname)
 * Returns the value of the passed \a pname for the given \a shader.
 * \a pname must be one of \c{Context3D.SHADER_TYPE}, \c Context3D.DELETE_STATUS and
 * \c{Context3D.COMPILE_STATUS}.
 */
/*!
 * \internal
 */
int CanvasContext::getShaderParameter(CanvasShader *shader, glEnums pname)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(shader:" << shader
                                << ", pname:"<< glEnumToString(pname)
                                << ")";
    if (!shader)
        return 0;

    switch (pname) {
    case SHADER_TYPE: {
        GLint shaderType = 0;
        glGetShaderiv( shader->qOGLShader()->shaderId(), GL_SHADER_TYPE, &shaderType);
        return shaderType;
    }
    case DELETE_STATUS: {
        bool isDeleted = !shader->isAlive();
        if (m_logAllCalls) qDebug() << "    getShaderParameter returns " << isDeleted;
        return (isDeleted ? GL_TRUE : GL_FALSE);
    }
    case COMPILE_STATUS: {
        bool isCompiled = shader->qOGLShader()->isCompiled();
        if (m_logAllCalls) qDebug() << "    getShaderParameter returns " << isCompiled;
        return (isCompiled ? GL_TRUE : GL_FALSE);
    }
    default: {
        if (m_logAllErrors) qDebug() << "getShaderParameter() : UNSUPPORTED parameter name "
                                     << glEnumToString(pname);
        return 0;
    }
    }
}

/*!
 * \qmlmethod Buffer3D Context3D::createBuffer()
 * Creates a Buffer3D object and initializes it with a buffer object name as if \c glGenBuffers()
 * was called.
 */
/*!
 * \internal
 */
CanvasBuffer *CanvasContext::createBuffer()
{
    CanvasBuffer *newBuffer = new CanvasBuffer(this);
    m_idToCanvasBufferMap[newBuffer->id()] = newBuffer;
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__ << ":" << newBuffer;

    // Returning a pointer to QObject that has parent set
    // -> V4VM should respect this and ownership should remain with this class
    return newBuffer;
}

/*!
 * \qmlmethod UniformLocation Context3D::getUniformLocation(Program3D program, string name)
 * Returns UniformLocation object that represents the location of a specific uniform variable
 * with the given \a name within the given \a program object.
 * Returns \c null if name doesn't correspond to a uniform variable.
 */
/*!
 * \internal
 */
CanvasUniformLocation *CanvasContext::getUniformLocation(CanvasProgram *program,
                                                         const QString &name)
{
    if (!program) {
        if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                    << "(program:" << program
                                    << ", name:" << name
                                    << "):-1";
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ": INVALID Program3D reference " << program;
        return 0;
    }

    int index = program->uniformLocation(name);
    if (index < 0) {
        return 0;
    }

    CanvasUniformLocation *location = new CanvasUniformLocation(index, this);
    location->setName(name);
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(program:" << program
                                << ", name:" << name
                                << "):" << location;

    // Returning a pointer to QObject that has parent set
    // -> V4VM should respect this and ownership should remain with this class
    return location;
}

/*!
 * \qmlmethod int Context3D::getAttribLocation(Program3D program, string name)
 * Returns location of the given attribute variable \a name in the given \a program.
 */
/*!
 * \internal
 */
int CanvasContext::getAttribLocation(CanvasProgram *program, const QString &name)
{
    if (!program) {
        if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                    << "(" << program
                                    << ", name:" << name
                                    << "):-1";
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ": INVALID Program3D reference " << program;
        return -1;
    } else {
        if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                    << "(program:" << program
                                    << ", name:" << name
                                    << "):" << program->attributeLocation(name);
    }

    return program->attributeLocation(name);
}

/*!
 * \qmlmethod void Context3D::bindAttribLocation(Program3D program, int index, string name)
 * Binds the attribute \a index with the attribute variable \a name in the given \a program.
 */
/*!
 * \internal
 */
void CanvasContext::bindAttribLocation(CanvasProgram *program, int index, const QString &name)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(program:" <<program
                                << ", index:" << index
                                << ", name:" << name
                                << ")";
    if (!program) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ": INVALID Program3D reference " << program;
        return;
    }

    program->bindAttributeLocation(index, name);
}

/*!
 * \qmlmethod void Context3D::enableVertexAttribArray(int index)
 * Enables the vertex attribute array at \a index.
 */
/*!
 * \internal
 */
void CanvasContext::enableVertexAttribArray(int index)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(index:" << index
                                << ")";
    glEnableVertexAttribArray(index);
}

/*!
 * \qmlmethod void Context3D::disableVertexAttribArray(int index)
 * Disables the vertex attribute array at \a index.
 */
/*!
 * \internal
 */
void CanvasContext::disableVertexAttribArray(int index)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(index:" << index
                                << ")";
    glDisableVertexAttribArray(index);
}

/*!
 * \qmlmethod void Context3D::uniformMatrix2fv(UniformLocation uniformLocation, bool transpose, Float32Array value)
 * Converts the float array given in \a value to a 2x2 matrix and sets it to the given
 * uniform at \a uniformLocation. Applies \a transpose if set to \c{true}.
 */
/*!
 * \internal
 */
void CanvasContext::uniformMatrix2fv(CanvasUniformLocation *uniformLocation, bool transpose,
                                     CanvasFloat32Array *value)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(uniformLocation:" << uniformLocation
                                << ", transpose:" << transpose
                                << ", value:" << value
                                <<")";
    if (!m_currentProgram || !uniformLocation || !value)
        return;
    if (m_logAllCalls) qDebug() << "    numMatrices:" << (value->length() / 4);

    int location = uniformLocation->id();
    float *arrayData = (float *)value->rawDataCptr();
    int numMatrices = value->length() / 4;

    glUniformMatrix2fv(location, numMatrices, transpose, arrayData);
}

/*!
 * \qmlmethod void Context3D::uniformMatrix3fv(UniformLocation uniformLocation, bool transpose, Float32Array value)
 * Converts the float array given in \a value to a 3x3 matrix and sets it to the given
 * uniform at \a uniformLocation. Applies \a transpose if set to \c{true}.
 */
/*!
 * \internal
 */
void CanvasContext::uniformMatrix3fv(CanvasUniformLocation *uniformLocation, bool transpose,
                                     CanvasFloat32Array *value)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(uniformLocation:" << uniformLocation
                                << ", transpose:" << transpose
                                << ", value:" << value
                                <<")";
    if (!m_currentProgram || !uniformLocation || !value)
        return;
    if (m_logAllCalls) qDebug() << "    numMatrices:" << (value->length() / 9);

    int location = uniformLocation->id();
    float *arrayData = (float *) value->rawDataCptr();
    int numMatrices = value->length() / 9;

    glUniformMatrix3fv(location, numMatrices, transpose, arrayData);
}

/*!
 * \qmlmethod void Context3D::uniformMatrix4fv(UniformLocation uniformLocation, bool transpose, Float32Array value)
 * Converts the float array given in \a value to a 4x4 matrix and sets it to the given
 * uniform at \a uniformLocation. Applies \a transpose if set to \c{true}.
 */
/*!
 * \internal
 */
void CanvasContext::uniformMatrix4fv(CanvasUniformLocation *uniformLocation, bool transpose,
                                     CanvasFloat32Array *value)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(uniformLocation:" << uniformLocation
                                << ", transpose:" << transpose
                                << ", value:" << value
                                << ")";
    if (!m_currentProgram || !uniformLocation || !value)
        return;

    if (m_logAllCalls) qDebug() << "    numMatrices:" << (value->length() / 16);
    int location = uniformLocation->id();
    float *arrayData = (float *)value->rawDataCptr();
    int numMatrices = value->length() / 16;

    glUniformMatrix4fv(location, numMatrices, transpose, arrayData);
}

/*!
 * \qmlmethod void Context3D::uniformMatrix4fva(UniformLocation uniformLocation, bool transpose, list<variant> value)
 * Converts the float array given in \a value to a 4x4 matrix and sets it to the given
 * uniform at \a uniformLocation. Applies \a transpose if set to \c{true}.
 */
/*!
 * \internal
 */
void CanvasContext::uniformMatrix4fva(CanvasUniformLocation *uniformLocation, bool transpose,
                                      QVariantList value)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(uniformLocation:" << uniformLocation
                                << ", transpose:" << transpose
                                << ", value:" << value
                                << ")";
    if (!m_currentProgram || !uniformLocation)
        return;

    int location = uniformLocation->id();
    int size = value.count();
    float *arrayData = new float[size];
    int numMatrices = size / 16;

    ArrayUtils::fillFloatArrayFromVariantList(value, arrayData);

    glUniformMatrix4fv(location, numMatrices, transpose, arrayData);

    delete [] arrayData;
}

/*!
 * \qmlmethod void Context3D::uniformMatrix3fva(UniformLocation uniformLocation, bool transpose, list<variant> value)
 * Converts the float array given in \a value to a 3x3 matrix and sets it to the given
 * uniform at \a uniformLocation. Applies \a transpose if set to \c{true}.
 */
/*!
 * \internal
 */
void CanvasContext::uniformMatrix3fva(CanvasUniformLocation *uniformLocation, bool transpose,
                                      QVariantList value)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(uniformLocation:" << uniformLocation
                                << ", transpose:" << transpose
                                << ", value:" << value
                                << ")";
    if (!m_currentProgram || !uniformLocation)
        return;

    int location = uniformLocation->id();
    int size = value.count();
    float *arrayData = new float[size];
    int numMatrices = size / 9;

    ArrayUtils::fillFloatArrayFromVariantList(value, arrayData);

    glUniformMatrix3fv(location, numMatrices, transpose, arrayData);

    delete [] arrayData;
}

/*!
 * \qmlmethod void Context3D::uniformMatrix2fva(UniformLocation uniformLocation, bool transpose, list<variant> value)
 * Converts the float array given in \a value to a 2x2 matrix and sets it to the given
 * uniform at \a uniformLocation. Applies \a transpose if set to \c{true}.
 */
/*!
 * \internal
 */
void CanvasContext::uniformMatrix2fva(CanvasUniformLocation *uniformLocation, bool transpose,
                                      QVariantList value)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(uniformLocation:" << uniformLocation
                                << ", transpose:" << transpose
                                << ", value:" << value
                                << ")";

    if (!m_currentProgram || !uniformLocation)
        return;

    int location = uniformLocation->id();
    int size = value.count();
    float *arrayData = new float[size];
    int numMatrices = size / 4;

    ArrayUtils::fillFloatArrayFromVariantList(value, arrayData);

    glUniformMatrix2fv(location, numMatrices, transpose, arrayData);

    delete [] arrayData;
}

/*!
 * \qmlmethod void Context3D::vertexAttribPointer(int indx, int size, glEnums type, bool normalized, int stride, long offset)
 * Sets the currently bound array buffer to the vertex attribute at the index passed at \a indx.
 * \a size is the number of components per attribute. \a stride specifies the byte offset between
 * consecutive vertex attributes. \a offset specifies the byte offset to the first vertex attribute
 * in the array. If int values should be normalized, set \a normalized to \c{true}.
 */
/*!
 * \internal
 */
void CanvasContext::vertexAttribPointer(int indx, int size, glEnums type,
                                        bool normalized, int stride, long offset)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(indx:" << indx
                                << ", size: " << size
                                << ", type:" << glEnumToString(type)
                                << ", normalized:" << normalized
                                << ", stride:" << stride
                                << ", offset:" << offset
                                << ")";

    glVertexAttribPointer(indx, size, GLenum(type), normalized, stride, (GLvoid *)offset);
}


/*!
 * \qmlmethod void Context3D::bufferData(glEnums target, long size, glEnums usage)
 * Sets the size of the \a target buffer to \a size. Target buffer must be either
 * \c Context3D.ARRAY_BUFFER or \c{Context3D.ELEMENT_ARRAY_BUFFER}. \a usage sets the usage pattern
 * of the data, and must be one of \c{Context3D.STREAM_DRAW}, \c{Context3D.STATIC_DRAW}, or
 * \c{Context3D.DYNAMIC_DRAW}.
 */
/*!
 * \internal
 */
void CanvasContext::bufferData(glEnums target, long size, glEnums usage)
{
    // TODO: Sort this out, it doesn't follow any reason or rhyme...
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(target:" << glEnumToString(target)
                                << ", size:" << size
                                << ", usage:" << glEnumToString(usage)
                                << ")";

    CanvasTypedArray *tempArray;
    switch (target) {
    case ARRAY_BUFFER:
        if (!m_currentArrayBuffer) {
            if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                         << ":INVALID_OPERATION called with no ARRAY_BUFFER bound";
            m_error = INVALID_OPERATION;
            return;
        }

        tempArray = createTypedArray(FLOAT, size);
        break;
    case ELEMENT_ARRAY_BUFFER:
        if (!m_currentElementArrayBuffer) {
            if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                         << ":INVALID_OPERATION called with no "
                                         << "ELEMENT_ARRAY_BUFFER bound";
            m_error = INVALID_OPERATION;
            return;
        }
        tempArray = createTypedArray(UNSIGNED_SHORT, size);
        break;
    default:
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_ENUM unknown target";
        m_error = INVALID_ENUM;
        return;
    }

    glBufferData(GLenum(target), size, (GLvoid *)tempArray->rawDataCptr(), GLenum(usage));
    delete tempArray;
}

/*!
 * \internal
 */
CanvasTypedArray *CanvasContext::createTypedArray(glEnums dataType, long size)
{
    switch (dataType) {
    case BYTE:
        return new CanvasInt8Array(size, this);
    case UNSIGNED_BYTE:
        return new CanvasUint8Array(size, this);
    case SHORT:
        return new CanvasInt16Array(size, this);
    case UNSIGNED_SHORT:
        return new CanvasUint16Array(size, this);
    case INT:
        return new CanvasInt32Array(size, this);
    case UNSIGNED_INT:
        return new CanvasUint32Array(size, this);
    case FLOAT:
        return new CanvasFloat32Array(size, this);
    default:
        return 0;
    }
}

/*!
 * \qmlmethod void Context3D::bufferData(glEnums target, TypedArray typedArray, glEnums usage)
 * Writes the data held in \a typedArray to the \a target buffer. Target buffer must be either
 * \c Context3D.ARRAY_BUFFER or \c{Context3D.ELEMENT_ARRAY_BUFFER}. \a usage sets the usage pattern
 * of the data, and must be one of \c{Context3D.STREAM_DRAW}, \c{Context3D.STATIC_DRAW}, or
 * \c{Context3D.DYNAMIC_DRAW}.
 */
/*!
 * \internal
 */
void CanvasContext::bufferData(glEnums target, CanvasTypedArray *typedArray, glEnums usage)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(target:" << glEnumToString(target)
                                << ", typedArray:" << typedArray
                                << ", usage:" << glEnumToString(usage)
                                << ")";

    if (!typedArray) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ": INVALID_VALUE called with null data";
        m_error = INVALID_VALUE;
        return;
    }

    if (target != ARRAY_BUFFER && target != ELEMENT_ARRAY_BUFFER) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_ENUM target must be either ARRAY_BUFFER"
                                     << " or ELEMENT_ARRAY_BUFFER.";
        m_error = INVALID_ENUM;
        return;
    }

    glBufferData(GLenum(target), typedArray->byteLength(), (GLvoid *)typedArray->rawDataCptr(),
                 GLenum(usage));
}

/*!
 * \qmlmethod void Context3D::bufferData(glEnums target, ArrayBuffer data, glEnums usage)
 * Writes the \a data to the \a target buffer. Target buffer must be either
 * \c Context3D.ARRAY_BUFFER or \c{Context3D.ELEMENT_ARRAY_BUFFER}. \a usage sets the usage pattern
 * of the data, and must be one of \c{Context3D.STREAM_DRAW}, \c{Context3D.STATIC_DRAW}, or
 * \c{Context3D.DYNAMIC_DRAW}.
 */
/*!
 * \internal
 */
void CanvasContext::bufferData(glEnums target, CanvasArrayBuffer &data, glEnums usage)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(target:" << glEnumToString(target)
                                << ", data: "
                                << "usage:" << glEnumToString(usage)
                                << ")";

    if (target != ARRAY_BUFFER && target != ELEMENT_ARRAY_BUFFER) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_ENUM target must be either ARRAY_BUFFER"
                                     << " or ELEMENT_ARRAY_BUFFER.";
        m_error = INVALID_ENUM;
        return;
    }

    glBufferData(GLenum(target), data.byteLength(), (GLvoid*)data.rawData(), GLenum(usage));
}

/*!
 * \qmlmethod void Context3D::bufferSubData(glEnums target, int offset, TypedArray typedArray)
 * Writes data starting from \a offset held in \a typedArray to the \a target buffer. Target
 * buffer must be either \c Context3D.ARRAY_BUFFER or \c{Context3D.ELEMENT_ARRAY_BUFFER}.
 */
/*!
 * \internal
 */
void CanvasContext::bufferSubData(glEnums target, int offset, CanvasTypedArray *typedArray)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(target:" << glEnumToString(target)
                                << ", offset:"<< offset
                                << ", typedArray:" << typedArray
                                << ")";

    if (target != ARRAY_BUFFER && target != ELEMENT_ARRAY_BUFFER) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_ENUM target must be either ARRAY_BUFFER"
                                     << " or ELEMENT_ARRAY_BUFFER.";
        m_error = INVALID_ENUM;
        return;
    }

    if (!typedArray) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ": INVALID_VALUE called with null data";
        m_error = INVALID_VALUE;
        return;
    }

    glBufferSubData(GLenum(target), offset, typedArray->byteLength(),
                    (GLvoid*)typedArray->rawDataCptr());
}

/*!
 * \qmlmethod void Context3D::bufferSubData(glEnums target, int offset, ArrayBuffer data)
 * Writes \a data starting from \a offset to the \a target buffer. Target
 * buffer must be either \c Context3D.ARRAY_BUFFER or \c{Context3D.ELEMENT_ARRAY_BUFFER}.
 */
/*!
 * \internal
 */
void CanvasContext::bufferSubData(glEnums target, int offset, CanvasArrayBuffer &data)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(target:" << glEnumToString(target)
                                << ", offset:"<< offset
                                << ", data.byteLength:" << data.byteLength()
                                << ")";

    if (target != ARRAY_BUFFER && target != ELEMENT_ARRAY_BUFFER) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_ENUM target must be either ARRAY_BUFFER"
                                     << " or ELEMENT_ARRAY_BUFFER.";
        m_error = INVALID_ENUM;
        return;
    }

    glBufferSubData(GLenum(target), offset, data.byteLength(), (GLvoid*)data.rawData());
}

/*!
 * \qmlmethod variant Context3D::getBufferParameter(glEnums target, glEnums pname)
 * Returns the value for the passed \a pname of the \a target. Target
 * must be either \c Context3D.ARRAY_BUFFER or \c{Context3D.ELEMENT_ARRAY_BUFFER}. pname must be
 * either \c Context3D.BUFFER_SIZE or \c{Context3D.BUFFER_USAGE}.
 */
/*!
 * \internal
 */
QVariant CanvasContext::getBufferParameter(glEnums target, glEnums pname)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(target:" << glEnumToString(target)
                                << ", pname" << glEnumToString(pname)
                                << ")";

    if (target != ARRAY_BUFFER && target != ELEMENT_ARRAY_BUFFER) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_ENUM target must be either ARRAY_BUFFER"
                                     << " or ELEMENT_ARRAY_BUFFER.";
        m_error = INVALID_ENUM;
        return 0;
    }

    switch (pname) {
    case BUFFER_SIZE:
    case BUFFER_USAGE:
        GLint data;
        glGetBufferParameteriv(GLenum(target), GLenum(pname), &data);

        // TODO: What happens here regarding ownership?
        return QVariant(data);
    default:
        if (m_logAllErrors) qDebug() << "getBufferParameter() : UNKNOWN pname";
        m_error = INVALID_ENUM;
        return 0;
    }

    return 0;
}

/*!
 * \qmlmethod bool Context3D::isBuffer(Object anyObject)
 * Returns \c true if the given \a anyObect is a valid Buffer3D object and \c false otherwise.
 */
/*!
 * \internal
 */
bool CanvasContext::isBuffer(QObject *anyObject)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(anyObject:" << anyObject
                                << ")";
    if (!anyObject)
        return false;

    QString className = QString(anyObject->metaObject()->className());
    if (className != "CanvasBuffer")
        return false;

    CanvasBuffer *buffer = static_cast<CanvasBuffer *>(anyObject);
    if (!buffer->isAlive())
        return false;

    return glIsBuffer(buffer->id());
}

/*!
 * \qmlmethod void Context3D::deleteBuffer(Buffer3D buffer)
 * Deletes the \a buffer. Has no effect if the Buffer3D object has been deleted already.
 */
/*!
 * \internal
 */
void CanvasContext::deleteBuffer(CanvasBuffer *buffer)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(buffer:" << buffer
                                << ")";
    if (!buffer) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ": Called with null buffer target";
        return;
    }

    m_idToCanvasBufferMap.remove(buffer->id());
    buffer->del();
}

/*!
 * \qmlmethod glEnums Context3D::getError()
 * Returns the error value, if any.
 */
/*!
 * \internal
 */
CanvasContext::glEnums CanvasContext::getError()
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__;
    glEnums retVal = m_error;
    m_error = NO_ERROR;
    if (retVal == NO_ERROR)
        retVal = glEnums(glGetError());
    return retVal;
}

/*!
 * \qmlmethod variant Context3D::getParameter(glEnums pname)
 * Returns the value for the given \a pname. pname must be one of \c{Context3D.RED_BITS},
 * \c{Context3D.GREEN_BITS}, \c{Context3D.BLUE_BITS}, \c{Context3D.ALPHA_BITS},
 * \c{Context3D.DEPTH_BITS}, \c{Context3D.STENCIL_BITS}, \c{Context3D.MAX_TEXTURE_IMAGE_UNITS},
 * \c{Context3D.MAX_VERTEX_TEXTURE_IMAGE_UNITS}, \c{Context3D.MAX_TEXTURE_SIZE},
 * \c{Context3D.MAX_CUBE_MAP_TEXTURE_SIZE}, \c{Context3D.MAX_VERTEX_UNIFORM_VECTORS}
 * (not supported in OpenGL ES2), \c{Context3D.RENDERER}, \c{Context3D.SHADING_LANGUAGE_VERSION},
 * \c{Context3D.VENDOR}, \c{Context3D.VERSION}, \c{Context3D.UNMASKED_VENDOR_WEBGL}, or
 * \c{Context3D.UNMASKED_RENDERER_WEBGL}.
 */
/*!
 * \internal
 */
QVariant CanvasContext::getParameter(glEnums pname)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "( pname:" << glEnumToString(pname)
                                << ")";
    GLint value;

    switch (pname) {
    // INTEGER PARAMETERS
    case RED_BITS:
        // Intentional flow through
    case GREEN_BITS:
        // Intentional flow through
    case BLUE_BITS:
        // Intentional flow through
    case ALPHA_BITS:
        // Intentional flow through
    case DEPTH_BITS:
        // Intentional flow through
    case STENCIL_BITS:
        // Intentional flow through
    case MAX_TEXTURE_IMAGE_UNITS:
        // Intentional flow through
    case MAX_VERTEX_TEXTURE_IMAGE_UNITS:
        // Intentional flow through
    case MAX_TEXTURE_SIZE:
        // Intentional flow through
    case MAX_CUBE_MAP_TEXTURE_SIZE: {
        glGetIntegerv(pname, &value);
        return QVariant::fromValue(value);
    }
#if !defined(QT_OPENGL_ES_2)
    case MAX_VERTEX_UNIFORM_VECTORS: {
        glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &value);
        if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__ << "():" << value;
        return QVariant::fromValue(value);
    }
#endif
        // STRING PARAMETERS
    case RENDERER:
        // Intentional flow through
    case SHADING_LANGUAGE_VERSION:
        // Intentional flow through
    case VENDOR:
        // Intentional flow through
    case VERSION: {
        const GLubyte *text = glGetString(pname);
        QString qtext = QString::fromLatin1((const char *)text);
        if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__ << "():" << qtext;
        return qtext;
        break;
    }
    case UNMASKED_VENDOR_WEBGL: {
        const GLubyte *text = glGetString(GL_VENDOR);
        QString qtext = QString::fromLatin1((const char *)text);
        if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__ << "():" << qtext;
        return qtext;
    }
    case UNMASKED_RENDERER_WEBGL: {
        const GLubyte *text = glGetString(GL_VENDOR);
        QString qtext = QString::fromLatin1((const char *)text);
        if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__ << "():" << qtext;
        return qtext;
    }
    default: {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << "(): UNIMPLEMENTED PARAMETER NAME" << glEnumToString(pname);
        return QVariant::fromValue(0);
    }
    }

    // TODO: Check if this handles ownership correctly
    return QVariant::fromValue(QString("NOT SUPPORTED YET"));
}

/*!
 * \qmlmethod string Context3D::getShaderInfoLog(Shader3D shader)
 * Returns the info log string of the given \a shader.
 */
/*!
 * \internal
 */
QString CanvasContext::getShaderInfoLog(CanvasShader *shader) const
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(shader:" << shader
                                << ")";
    if (!shader)
        return QString();

    // Returning a copy, V4VM takes ownership
    return shader->qOGLShader()->log();
}

/*!
 * \qmlmethod string Context3D::getProgramInfoLog(Program3D program)
 * Returns the info log string of the given \a program.
 */
/*!
 * \internal
 */
QString CanvasContext::getProgramInfoLog(CanvasProgram *program) const
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(program:" << program
                                << ")";
    if (!program)
        return QString();

    // Returning a copy, V4VM takes ownership
    return program->log();
}

/*!
 * \qmlmethod void Context3D::bindBuffer(glEnums target, Buffer3D buffer)
 * Binds the given \a buffer to the given \a target. Target must be either
 * \c Context3D.ARRAY_BUFFER or \c{Context3D.ELEMENT_ARRAY_BUFFER}. If the \a buffer given is
 * \c{null}, the current buffer bound to the target is unbound.
 */
/*!
 * \internal
 */
void CanvasContext::bindBuffer(glEnums target, CanvasBuffer *buffer)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(target:" << glEnumToString(target)
                                << ", buffer:" <<buffer<< ")";

    if (target != ARRAY_BUFFER && target != ELEMENT_ARRAY_BUFFER) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_ENUM target must be either ARRAY_BUFFER or"
                                     << "ELEMENT_ARRAY_BUFFER.";
        m_error = INVALID_ENUM;
        return;
    }

    if (buffer) {
        if (target == ARRAY_BUFFER) {
            if (buffer->target() == CanvasBuffer::UNINITIALIZED)
                buffer->setTarget(CanvasBuffer::ARRAY_BUFFER);
            if (buffer->target() != CanvasBuffer::ARRAY_BUFFER) {
                if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                             << ":INVALID_OPERATION can't bind "
                                             << "ELEMENT_ARRAY_BUFFER as ARRAY_BUFFER";
                m_error = INVALID_OPERATION;
                return;
            }
            m_currentArrayBuffer = buffer;
            glBindBuffer(GLenum(target), buffer->id());
        } else {
            if (buffer->target() == CanvasBuffer::UNINITIALIZED)
                buffer->setTarget(CanvasBuffer::ELEMENT_ARRAY_BUFFER);
            if (buffer->target() != CanvasBuffer::ELEMENT_ARRAY_BUFFER) {
                if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                             << ":INVALID_OPERATION can't bind "
                                             << "ARRAY_BUFFER as ELEMENT_ARRAY_BUFFER";
                m_error = INVALID_OPERATION;
                return;
            }
            m_currentElementArrayBuffer = buffer;
            glBindBuffer(GLenum(target), buffer->id());
        }
    } else {
        glBindBuffer(GLenum(target), 0);
    }

}

// TODO: Is this function useful? We don't offer a way to query the status.
/*!
 * \qmlmethod void Context3D::validateProgram(Program3D program)
 * Validates the given \a program. The validation status is stored into the state of the shader
 * program container in \a program.
 */
/*!
 * \internal
 */
void CanvasContext::validateProgram(CanvasProgram *program)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(program:" << program << ")";
    if (program)
        program->validateProgram();
}

/*!
 * \qmlmethod void Context3D::useProgram(Program3D program)
 * Installs the given \a program as a part of the current rendering state.
 */
/*!
 * \internal
 */
void CanvasContext::useProgram(CanvasProgram *program)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(program:" << program << ")";
    // TODO: Check if this is ok
    m_currentProgram = program;
    if (!program || !program->isLinked())
        return;
    program->bind();
}

/*!
 * \qmlmethod void Context3D::clear(glEnums flags)
 * Clears the given \a flags.
 */
/*!
 * \internal
 */
void CanvasContext::clear(glEnums flags)
{
    if (m_logAllCalls) {
        QString flagStr;
        if (flags && COLOR_BUFFER_BIT != 0)
            flagStr.append(" COLOR_BUFFER_BIT ");

        if (flags && DEPTH_BUFFER_BIT != 0)
            flagStr.append(" DEPTH_BUFFER_BIT ");

        if (flags && STENCIL_BUFFER_BIT != 0)
            flagStr.append(" STENCIL_BUFFER_BIT ");

        qDebug() << "Context3D::" << __FUNCTION__ << "(flags:" << flagStr << ")";
    }

    glClear(flags);
}

/*!
 * \qmlmethod void Context3D::cullFace(glEnums mode)
 * Sets the culling to \a mode. Must be one of \c{Context3D.FRONT}, \c{Context3D.BACK},
 * or \c{Context3D.FRONT_AND_BACK}. Defaults to \c{Context3D.BACK}.
 */
/*!
 * \internal
 */
void CanvasContext::cullFace(glEnums mode)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(mode:" << glEnumToString(mode) << ")";
    glCullFace(mode);
}

/*!
 * \qmlmethod void Context3D::frontFace(glEnums mode)
 * Sets the front face drawing to \a mode. Must be either \c Context3D.CW
 * or \c{Context3D.CCW}. Defaults to \c{Context3D.CCW}.
 */
/*!
 * \internal
 */
void CanvasContext::frontFace(glEnums mode)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(mode:" << glEnumToString(mode) << ")";
    glFrontFace(mode);
}

/*!
 * \qmlmethod void Context3D::depthMask(bool flag)
 * Enables or disables the depth mask based on \a flag. Defaults to \c{true}.
 */
/*!
 * \internal
 */
void CanvasContext::depthMask(bool flag)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(flag:" << flag << ")";
    if (flag)
        glDepthMask(GL_TRUE);
    else
        glDepthMask(GL_FALSE);
}

// TODO: Why are all the enums for this commented out?
/*!
 * \qmlmethod void Context3D::depthFunc(glEnums func)
 * Sets the depth function to \a func. Must be one of \c{Context3D.NEVER}, \c{Context3D.LESS},
 * \c{Context3D.EQUAL}, \c{Context3D.LEQUAL}, \c{Context3D.GREATER}, \c{Context3D.NOTEQUAL},
 * \c{Context3D.GEQUAL}, or \c{Context3D.ALWAYS}. Defaults to \c{Context3D.LESS}.
 */
/*!
 * \internal
 */
void CanvasContext::depthFunc(glEnums func)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(func:" << glEnumToString(func) << ")";
    glDepthFunc(func);
}

/*!
 * \qmlmethod void Context3D::depthRange(float zNear, float zFar)
 * Sets the depth range between \a zNear and \a zFar. Values are clamped to \c{[0, 1]}. \a zNear
 * must be less or equal to \a zFar. zNear Range defaults to \c{[0, 1]}.
 */
/*!
 * \internal
 */
void CanvasContext::depthRange(float zNear, float zFar)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(zNear:" << zNear
                                << ", zFar:" << zFar <<  ")";
    glDepthRangef(GLclampf(zNear), GLclampf(zFar));
}

/*!
 * \qmlmethod void Context3D::clearStencil(int stencil)
 * Sets the clear value for the stencil buffer to \a stencil. Defaults to \c{0}.
 */
/*!
 * \internal
 */
void CanvasContext::clearStencil(int stencil)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__ << "(stencil:" << stencil << ")";
    glClearStencil(stencil);
}

/*!
 * \qmlmethod void Context3D::colorMask(bool maskRed, bool maskGreen, bool maskBlue, bool maskAlpha)
 * Enables or disables the writing of colors to the frame buffer based on \a maskRed, \a maskGreen,
 * \a maskBlue and \a maskAlpha. All default to \c{true}.
 */
/*!
 * \internal
 */
void CanvasContext::colorMask(bool maskRed, bool maskGreen, bool maskBlue, bool maskAlpha)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(maskRed:" << maskRed
                                << ", maskGreen:" << maskGreen
                                << ", maskBlue:" << maskBlue
                                << ", maskAlpha:" << maskAlpha  <<  ")";
    glColorMask(maskRed, maskGreen, maskBlue, maskAlpha);
}

/*!
 * \qmlmethod void Context3D::clearDepth(float depth)
 * Sets the clear value for the depth buffer to \a depth. Must be between \c{[0, 1]}. Defaults
 * to \c{1}.
 */
/*!
 * \internal
 */
void CanvasContext::clearDepth(float depth)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__ << "(depth:" << depth << ")";
    glClearDepthf(depth);
}

/*!
 * \qmlmethod void Context3D::clearColor(float red, float green, float blue, float alpha)
 * Sets the clear values for the color buffers with \a red, \a green, \a blue and \a alpha. Values
 * must be between \c{[0, 1]}. All default to \c{0}.
 */
/*!
 * \internal
 */
void CanvasContext::clearColor(float red, float green, float blue, float alpha)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                <<  "(red:" << red
                                 << ", green:" << green
                                 << ", blue:" << blue
                                 << ", alpha:" << alpha << ")";
    glClearColor(red, green, blue, alpha);
}

/*!
 * \qmlmethod Context3D::viewport(int x, int y, int width, int height)
 * Defines the affine transformation from normalized x and y device coordinates to window
 * coordinates within the drawing buffer.
 * \a x defines the left edge of the viewport.
 * \a y defines the bottom edge of the viewport.
 * \a width defines the width of the viewport.
 * \a height defines the height of the viewport.
 */
/*!
 * \internal
 */
void CanvasContext::viewport(int x, int y, int width, int height)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                <<  "(x:" << x
                                 << ", y:" << y
                                 << ", width:" << width
                                 << ", height:" << height << ")";
    glViewport(x, y, width, height);
    m_glViewportRect.setX(x);
    m_glViewportRect.setY(y);
    m_glViewportRect.setWidth(width);
    m_glViewportRect.setHeight(height);
}

/*!
 * \qmlmethod void Context3D::drawArrays(glEnums mode, int first, int count)
 * Renders the geometric primitives held in the currently bound array buffer starting from \a first
 * up to \a count using \a mode for drawing. Mode can be one of:
 * \list
 * \li \c{Context3D.POINTS}
 * \li \c{Context3D.LINES}
 * \li \c{Context3D.LINE_LOOP}
 * \li \c{Context3D.LINE_STRIP}
 * \li \c{Context3D.TRIANGLES}
 * \li \c{Context3D.TRIANGLE_STRIP}
 * \li \c{Context3D.TRIANGLE_FAN}
 * \endlist
 */
/*!
 * \internal
 */
void CanvasContext::drawArrays(glEnums mode, int first, int count)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(mode:" << glEnumToString(mode)
                                << ", first:" << first
                                << ", count:" << count << ")";
    glDrawArrays(mode, first, count);
}

/*!
 * \qmlmethod void Context3D::drawElements(glEnums mode, int count, glEnums type, long offset)
 * Renders the number of geometric elements given in \a count held in the currently bound element
 * array buffer using \a mode for drawing. Mode can be one of:
 * \list
 * \li \c{Context3D.POINTS}
 * \li \c{Context3D.LINES}
 * \li \c{Context3D.LINE_LOOP}
 * \li \c{Context3D.LINE_STRIP}
 * \li \c{Context3D.TRIANGLES}
 * \li \c{Context3D.TRIANGLE_STRIP}
 * \li \c{Context3D.TRIANGLE_FAN}
 * \endlist
 *
 * \a type specifies the element type and can be one of:
 * \list
 * \li \c Context3D.UNSIGNED_BYTE
 * \li \c{Context3D.UNSIGNED_SHORT}
 * \endlist
 *
 * \a offset specifies the location where indices are stored.
 */
/*!
 * \internal
 */
void CanvasContext::drawElements(glEnums mode, int count, glEnums type, long offset)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(mode:" << glEnumToString(mode)
                                << ", count:" << count
                                << ", type:" << glEnumToString(type)
                                << ", offset:" << offset << ")";
    glDrawElements(GLenum(mode), count, GLenum(type), (GLvoid*)offset);
}

/*!
 * \qmlmethod void Context3D::readPixels(int x, int y, long width, long height, glEnums format, glEnums type, ArrayBufferView pixels)
 * Returns the pixel data in the rectangle specified by \a x, \a y, \a width and \a height of the
 * frame buffer in \a pixels using \a format (must be \c{Context3D.RGBA}) and \a type
 * (must be \c{Context3D.UNSIGNED_BYTE}).
 */
/*!
 * \internal
 */
void CanvasContext::readPixels(int x, int y, long width, long height, glEnums format, glEnums type,
                               CanvasArrayBufferView *pixels)
{
    if (format != RGBA) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_ENUM format must be RGBA.";
        m_error = INVALID_ENUM;
        return;
    }
    if (type != UNSIGNED_BYTE) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_ENUM type must be UNSIGNED_BYTE.";
        m_error = INVALID_ENUM;
        return;
    }
    if (!pixels) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_VALUE pixels was null.";
        m_error = INVALID_VALUE;
        return;
    }

    if (pixels->bufferType() != CanvasTypedArray::ARRAY_BUFFER_UINT_8) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_OPERATION pixels must be Uint8Array.";
        m_error = INVALID_OPERATION;
        return;
    }

    glReadPixels(x, y, width, height, format, type, pixels->rawDataCptr());
}

/*!
 * \qmlmethod ActiveInfo3D Context3D::getActiveAttrib(Program3D program, uint index)
 * Returns information about the given active attribute variable defined by \a index for the given
 * \a program.
 * \sa ActiveInfo3D
 */
/*!
 * \internal
 */
CanvasActiveInfo *CanvasContext::getActiveAttrib(CanvasProgram *program, uint index)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(program:" << program
                                << ", index:" << index << ")";

    char *name = new char[512];
    GLsizei length = 0;
    int size = 0;
    GLenum type = 0;
    glGetActiveAttrib(program->id(), index, 512, &length, &size, &type, name);
    QString strName(name);
    delete [] name;
    return new CanvasActiveInfo(size, CanvasContext::glEnums(type), strName);
}

/*!
 * \qmlmethod ActiveInfo3D Context3D::getActiveUniform(Program3D program, uint index)
 * Returns information about the given active uniform variable defined by \a index for the given
 * \a program.
 * \sa ActiveInfo3D
 */
/*!
 * \internal
 */
CanvasActiveInfo *CanvasContext::getActiveUniform(CanvasProgram *program, uint index)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(program:" << program
                                << ", index:" << index << ")";

    char *name = new char[512];
    GLsizei length = 0;
    int size = 0;
    GLenum type = 0;
    glGetActiveUniform(program->id(), index, 512, &length, &size, &type, name);
    QString strName(name);
    delete [] name;
    return new CanvasActiveInfo(size, CanvasContext::glEnums(type), strName);
}

/*!
 * \qmlmethod void Context3D::stencilFunc(glEnums func, int ref, uint mask)
 * Sets front and back function \a func and reference value \a ref for stencil testing.
 * Also sets the \a mask value that is ANDed with both reference and stored stencil value when
 * the test is done. \a func is initially set to \c{Context3D.ALWAYS} and can be one of:
 * \list
 * \li \c{Context3D.NEVER}
 * \li \c{Context3D.LESS}
 * \li \c{Context3D.LEQUAL}
 * \li \c{Context3D.GREATER}
 * \li \c{Context3D.GEQUAL}
 * \li \c{Context3D.EQUAL}
 * \li \c{Context3D.NOTEQUAL}
 * \li \c{Context3D.ALWAYS}
 * \endlist
 */
/*!
 * \internal
 */
void CanvasContext::stencilFunc(glEnums func, int ref, uint mask)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(func:" <<  glEnumToString(func)
                                << ", ref:" << ref
                                << ", mask:" << mask
                                << ")";

    glStencilFunc(GLenum(func), ref, mask);
}

/*!
 * \qmlmethod void Context3D::stencilFuncSeparate(glEnums face, glEnums func, int ref, uint mask)
 * Sets front and/or back (defined by \a face) function \a func and reference value \a ref for
 * stencil testing. Also sets the \a mask value that is ANDed with both reference and stored stencil
 * value when the test is done. \a face can be one of:
 * \list
 * \li \c{Context3D.FRONT}
 * \li \c{Context3D.BACK}
 * \li \c{Context3D.FRONT_AND_BACK}
 * \endlist
 * \a func is initially set to \c{Context3D.ALWAYS} and can be one of:
 * \list
 * \li \c{Context3D.NEVER}
 * \li \c{Context3D.LESS}
 * \li \c{Context3D.LEQUAL}
 * \li \c{Context3D.GREATER}
 * \li \c{Context3D.GEQUAL}
 * \li \c{Context3D.EQUAL}
 * \li \c{Context3D.NOTEQUAL}
 * \li \c{Context3D.ALWAYS}
 * \endlist
 */
/*!
 * \internal
 */
void CanvasContext::stencilFuncSeparate(glEnums face, glEnums func, int ref, uint mask)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(face:" <<  glEnumToString(face)
                                << ", func:" <<  glEnumToString(func)
                                << ", ref:" << ref
                                << ", mask:" << mask
                                << ")";
    glStencilFuncSeparate(GLenum(face), GLenum(func), ref, mask);
}

/*!
 * \qmlmethod void Context3D::stencilMask(uint mask)
 * Controls the front and back writing of individual bits in the stencil planes. \a mask defines
 * the bit mask to enable and disable writing of individual bits in the stencil planes.
 */
/*!
 * \internal
 */
void CanvasContext::stencilMask(uint mask)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(mask:" << mask
                                << ")";
    glStencilMask(mask);
}

/*!
 * \qmlmethod void Context3D::stencilMaskSeparate(glEnums face, uint mask)
 * Controls the front and/or back writing (defined by \a face) of individual bits in the stencil
 * planes. \a mask defines the bit mask to enable and disable writing of individual bits in the
 * stencil planes. \a face can be one of:
 * \list
 * \li \c{Context3D.FRONT}
 * \li \c{Context3D.BACK}
 * \li \c{Context3D.FRONT_AND_BACK}
 * \endlist
 */
/*!
 * \internal
 */
void CanvasContext::stencilMaskSeparate(glEnums face, uint mask)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(face:" <<  glEnumToString(face)
                                << ", mask:" << mask
                                << ")";
    glStencilMaskSeparate(GLenum(face), mask);
}

/*!
 * \qmlmethod void Context3D::stencilOp(glEnums sfail, glEnums zfail, glEnums zpass)
 * Sets the front and back stencil test actions for failing the test \a zfail and passing the test
 * \a zpass. \a sfail, \a zfail and \a zpass are initially set to \c{Context3D.KEEP} and can be one
 * of:
 * \list
 * \li \c{Context3D.KEEP}
 * \li \c{Context3D.ZERO}
 * \li \c{Context3D.GL_REPLACE}
 * \li \c{Context3D.GL_INCR}
 * \li \c{Context3D.GL_INCR_WRAP}
 * \li \c{Context3D.GL_DECR}
 * \li \c{Context3D.GL_DECR_WRAP}
 * \li \c{Context3D.GL_INVERT}
 * \endlist
 */
/*!
 * \internal
 */
void CanvasContext::stencilOp(glEnums sfail, glEnums zfail, glEnums zpass)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(sfail:" <<  glEnumToString(sfail)
                                << ", zfail:" <<  glEnumToString(zfail)
                                << ", zpass:" << glEnumToString(zpass)
                                << ")";
    glStencilOp(GLenum(sfail), GLenum(zfail), GLenum(zpass));
}

/*!
 * \qmlmethod void Context3D::stencilOpSeparate(glEnums face, glEnums fail, glEnums zfail, glEnums zpass)
 * Sets the front and/or back (defined by \a face) stencil test actions for failing the test
 * \a zfail and passing the test \a zpass. \a face can be one of:
 * \list
 * \li \c{Context3D.FRONT}
 * \li \c{Context3D.BACK}
 * \li \c{Context3D.FRONT_AND_BACK}
 * \endlist
 *
 * \a sfail, \a zfail and \a zpass are initially set to \c{Context3D.KEEP} and can be one of:
 * \list
 * \li \c{Context3D.KEEP}
 * \li \c{Context3D.ZERO}
 * \li \c{Context3D.GL_REPLACE}
 * \li \c{Context3D.GL_INCR}
 * \li \c{Context3D.GL_INCR_WRAP}
 * \li \c{Context3D.GL_DECR}
 * \li \c{Context3D.GL_DECR_WRAP}
 * \li \c{Context3D.GL_INVERT}
 * \endlist
 */
/*!
 * \internal
 */
void CanvasContext::stencilOpSeparate(glEnums face, glEnums fail, glEnums zfail, glEnums zpass)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(face:" <<  glEnumToString(face)
                                << ", fail:" <<  glEnumToString(fail)
                                << ", zfail:" <<  glEnumToString(zfail)
                                << ", zpass:" << glEnumToString(zpass)
                                << ")";
    glStencilOpSeparate(GLenum(face), GLenum(fail), GLenum(zfail), GLenum(zpass));
}


/*!
 * \qmlmethod void Context3D::vertexAttrib1fva(int indx, list<variant> values)
 * Sets the array of float values given in \a values to the generic vertex attribute index
 * specified by \a indx.
 */
/*!
 * \internal
 */
void CanvasContext::vertexAttrib1fva(uint indx, QVariantList values)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(indx" << indx
                                << ", values:" << values
                                << ")";
    if (!m_currentProgram)
        return;

    int size = values.count();
    float *arrayData = new float[size];

    ArrayUtils::fillFloatArrayFromVariantList(values, arrayData);

    glVertexAttrib1fv(indx, arrayData);

    delete [] arrayData;
}

/*!
 * \qmlmethod void Context3D::vertexAttrib2fva(int indx, list<variant> values)
 * Sets the array of float values given in \a values to the generic vertex attribute index
 * specified by \a indx.
 */
/*!
 * \internal
 */
void CanvasContext::vertexAttrib2fva(uint indx, QVariantList values)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(indx" << indx
                                << ", values:" << values
                                << ")";
    if (!m_currentProgram)
        return;

    int size = values.count();
    float *arrayData = new float[size];

    ArrayUtils::fillFloatArrayFromVariantList(values, arrayData);

    glVertexAttrib2fv(indx, arrayData);

    delete [] arrayData;
}

/*!
 * \qmlmethod void Context3D::vertexAttrib3fva(int indx, list<variant> values)
 * Sets the array of float values given in \a values to the generic vertex attribute index
 * specified by \a indx.
 */
/*!
 * \internal
 */
void CanvasContext::vertexAttrib3fva(uint indx, QVariantList values)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(indx" << indx
                                << ", values:" << values
                                << ")";
    if (!m_currentProgram)
        return;

    int size = values.count();
    float *arrayData = new float[size];

    ArrayUtils::fillFloatArrayFromVariantList(values, arrayData);

    glVertexAttrib3fv(indx, arrayData);

    delete [] arrayData;
}

/*!
 * \qmlmethod void Context3D::vertexAttrib4fva(int indx, list<variant> values)
 * Sets the array of float values given in \a values to the generic vertex attribute index
 * specified by \a indx.
 */
/*!
 * \internal
 */
void CanvasContext::vertexAttrib4fva(uint indx, QVariantList values)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(indx" << indx
                                << ", values:" << values
                                << ")";
    if (!m_currentProgram)
        return;

    int size = values.count();
    float *arrayData = new float[size];

    ArrayUtils::fillFloatArrayFromVariantList(values, arrayData);

    glVertexAttrib4fv(indx, arrayData);

    delete [] arrayData;
}

/*!
 * \qmlmethod int Context3D::getFramebufferAttachmentParameter(glEnums target, glEnums attachment, glEnums pname)
 * Returns information specified by \a pname about given \a attachment of a framebuffer object
 * bound to the given \a target.
 */
/*!
 * \internal
 */
int CanvasContext::getFramebufferAttachmentParameter(glEnums target, glEnums attachment,
                                                     glEnums pname)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(target" << glEnumToString(target)
                                << ", attachment:" << glEnumToString(attachment)
                                << ", pname:" << glEnumToString(pname)
                                << ")";
    GLint parameter;
    glGetFramebufferAttachmentParameteriv(target, attachment, pname, &parameter);
    return parameter;
}

/*!
 * \qmlmethod int Context3D::getRenderbufferParameter(glEnums target, glEnums pname)
 * Returns information specified by \a pname of a renderbuffer object
 * bound to the given \a target.
 */
/*!
 * \internal
 */
int CanvasContext::getRenderbufferParameter(glEnums target, glEnums pname)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(target" << glEnumToString(target)
                                << ", pname:" << glEnumToString(pname)
                                << ")";

    GLint parameter;
    glGetRenderbufferParameteriv(target, pname, &parameter);
    return parameter;
}

/*!
 * \qmlmethod variant Context3D::getTexParameter(glEnums target, glEnums pname)
 * Returns parameter specified by the \a pname of a texture object
 * bound to the given \a target. \a pname must be one of:
 * \list
 * \li \c{Context3D.TEXTURE_MAG_FILTER}
 * \li \c{Context3D.TEXTURE_MIN_FILTER}
 * \li \c{Context3D.TEXTURE_WRAP_S}
 * \li \c{Context3D.TEXTURE_WRAP_T}
 * \endlist
 */
/*!
 * \internal
 */
QVariant CanvasContext::getTexParameter(glEnums target, glEnums pname)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(target" << glEnumToString(target)
                                << ", pname:" << glEnumToString(pname)
                                << ")";

    GLint parameter = 0;
    if (!m_currentTexture) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_OPERATION No current texture bound";
        m_error = INVALID_OPERATION;
    } else if (!m_currentTexture->isAlive()) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_OPERATION Currently bound texture is deleted";
        m_error = INVALID_OPERATION;
    } else {
        switch (pname) {
        case TEXTURE_MAG_FILTER:
            // Intentional flow through
        case TEXTURE_MIN_FILTER:
            // Intentional flow through
        case TEXTURE_WRAP_S:
            // Intentional flow through
        case TEXTURE_WRAP_T:
            glGetTexParameteriv(target, pname, &parameter);
            break;
        default:
            // Intentional flow through
            if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                         << ":INVALID_ENUM invalid pname "<< glEnumToString(pname)
                                         << " must be one of: TEXTURE_MAG_FILTER, "
                                         << "TEXTURE_MIN_FILTER, TEXTURE_WRAP_S or TEXTURE_WRAP_T";
            m_error = INVALID_ENUM;
            break;
        }
    }

    return QVariant::fromValue(parameter);
}


/*!
 * \qmlmethod int Context3D::getVertexAttribOffset(int index, glEnums pname)
 * Returns the offset of the specified generic vertex attribute pointer \a index. \a pname must be
 * \c{Context3D.VERTEX_ATTRIB_ARRAY_POINTER}
 * \list
 * \li \c{Context3D.TEXTURE_MAG_FILTER}
 * \li \c{Context3D.TEXTURE_MIN_FILTER}
 * \li \c{Context3D.TEXTURE_WRAP_S}
 * \li \c{Context3D.TEXTURE_WRAP_T}
 * \endlist
 */
/*!
 * \internal
 */
uint CanvasContext::getVertexAttribOffset(uint index, glEnums pname)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(index" << index
                                << ", pname:" << glEnumToString(pname)
                                << ")";

    uint offset = 0;
    if (pname != VERTEX_ATTRIB_ARRAY_POINTER) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_ENUM pname must be "
                                     << "VERTEX_ATTRIB_ARRAY_POINTER";
        m_error = INVALID_ENUM;
        return 0;
    }

    if (index >= m_maxVertexAttribs) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_VALUE index must be smaller than "
                                     << m_maxVertexAttribs;
        m_error = INVALID_VALUE;
        return 0;
    }

    glGetVertexAttribPointerv(index, GLenum(pname), (GLvoid**) &offset);
    return offset;
}

/*!
 * \qmlmethod variant Context3D::getVertexAttrib(int index, glEnums pname)
 * Returns the requested parameter \a pname of the specified generic vertex attribute pointer
 * \a index. The type returned is dependent on the requested \a pname, as shown in the table:
 * \table
 * \header
 *   \li pname
 *   \li Returned Type
 * \row
 *   \li \c{Context3D.VERTEX_ATTRIB_ARRAY_BUFFER_BINDING}
 *   \li \c{CanvasBuffer}
 * \row
 *   \li \c{Context3D.VERTEX_ATTRIB_ARRAY_ENABLED}
 *   \li \c{boolean}
 * \row
 *   \li \c{Context3D.VERTEX_ATTRIB_ARRAY_SIZE}
 *   \li \c{int}
 * \row
 *   \li \c{Context3D.VERTEX_ATTRIB_ARRAY_STRIDE}
 *   \li \c{int}
 * \row
 *   \li \c{Context3D.VERTEX_ATTRIB_ARRAY_TYPE}
 *   \li \c{glEnums}
 * \row
 *   \li \c{Context3D.VERTEX_ATTRIB_ARRAY_NORMALIZED}
 *   \li \c{boolean}
 * \row
 *   \li \c{Context3D.CURRENT_VERTEX_ATTRIB}
 *   \li \c{sequence<float>} (with 4 elements)
 *  \endtable
 */
/*!
 * \internal
 */
QVariant CanvasContext::getVertexAttrib(uint index, glEnums pname)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(index" << index
                                << ", pname:" << glEnumToString(pname)
                                << ")";

    if (index >= MAX_VERTEX_ATTRIBS) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_VALUE index must be smaller than "
                                     << "MAX_VERTEX_ATTRIBS = " << MAX_VERTEX_ATTRIBS;
        m_error = INVALID_VALUE;
    } else {
        switch (pname) {
        case VERTEX_ATTRIB_ARRAY_BUFFER_BINDING: {
            GLint value = 0;
            glGetVertexAttribiv(index, GLenum(pname), &value);
            if (value == 0 || !m_idToCanvasBufferMap.contains(value))
                return QVariant();

            return QVariant::fromValue(m_idToCanvasBufferMap[value]);
        }
            break;
        case VERTEX_ATTRIB_ARRAY_ENABLED: {
            GLint value = 0;
            glGetVertexAttribiv(index, GLenum(pname), &value);
            return QVariant::fromValue( bool(value));
        }
            break;
        case VERTEX_ATTRIB_ARRAY_SIZE: {
            GLint value = 0;
            glGetVertexAttribiv(index, GLenum(pname), &value);
            return QVariant::fromValue(value);
        }
            break;
        case VERTEX_ATTRIB_ARRAY_STRIDE: {
            GLint value = 0;
            glGetVertexAttribiv(index, GLenum(pname), &value);
            return QVariant::fromValue(value);
        }
            break;
        case VERTEX_ATTRIB_ARRAY_TYPE: {
            GLint value = 0;
            glGetVertexAttribiv(index, GLenum(pname), &value);
            return QVariant::fromValue(value);
        }
        case VERTEX_ATTRIB_ARRAY_NORMALIZED: {
            GLint value = 0;
            glGetVertexAttribiv(index, GLenum(pname), &value);
            return QVariant::fromValue( bool(value));
        }
        case CURRENT_VERTEX_ATTRIB: {
            // TODO: Should be Float32Array
            GLfloat values[4];
            glGetVertexAttribfv(index, GLenum(pname), values);

            QList<float> floatList;
            floatList.push_back(values[0]);
            floatList.push_back(values[1]);
            floatList.push_back(values[2]);
            floatList.push_back(values[3]);
            return QVariant::fromValue(floatList);
        }
        default:
            if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                         << ":INVALID_ENUM index must be smaller than "
                                         << "MAX_VERTEX_ATTRIBS = " << MAX_VERTEX_ATTRIBS;
            m_error = INVALID_ENUM;
        }
    }

    return QVariant();
}

/*!
 * \qmlmethod variant Context3D::getUniform(Program3D program, UniformLocation3D location)
 * Returns the uniform value at the given \a location in the \a program.
 * The type returned is dependent on the uniform type, as shown in the table:
 * \table
 * \header
 *   \li Data Type
 *   \li Returned Type
 * \row
 *   \li boolean
 *   \li sequence<boolean> (with 1 element)
 * \row
 *   \li int
 *   \li sequence<int> (with 1 element)
 * \row
 *   \li float
 *   \li sequence<float> (with 1 element)
 * \row
 *   \li vec2
 *   \li sequence<float> (with 2 elements)
 * \row
 *   \li vec3
 *   \li sequence<float> (with 3 elements)
 * \row
 *   \li vec4
 *   \li sequence<float> (with 4 elements)
 * \row
 *   \li ivec2
 *   \li sequence<int> (with 2 elements)
 * \row
 *   \li ivec3
 *   \li sequence<int> (with 3 elements)
 * \row
 *   \li ivec4
 *   \li sequence<int> (with 4 elements)
 * \row
 *   \li bvec2
 *   \li sequence<boolean> (with 2 elements)
 * \row
 *   \li bvec3
 *   \li sequence<boolean> (with 3 elements)
 * \row
 *   \li bvec4
 *   \li sequence<boolean> (with 4 elements)
 * \row
 *   \li mat2
 *   \li sequence<float> (with 4 elements)
 * \row
 *   \li mat3
 *   \li sequence<float> (with 9 elements)
 * \row
 *   \li mat4
 *   \li sequence<float> (with 16 elements)
 * \row
 *   \li sampler2D
 *   \li sequence<int> (with 1 element)
 * \row
 *   \li samplerCube
 *   \li sequence<int> (with 1 element)
 *  \endtable
 */
/*!
 * \internal
 */
QVariant CanvasContext::getUniform(CanvasProgram *program, CanvasUniformLocation *location)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(program" << program
                                << ", location:" << location
                                << ")";

    if (!program) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_OPERATION No program was specified";
        m_error = INVALID_OPERATION;
    } else  if (!location) {
        if (m_logAllErrors) qDebug() << "Context3D::" << __FUNCTION__
                                     << ":INVALID_OPERATION No location was specified";
        m_error = INVALID_OPERATION;
    } else {
        uint programId = program->id();
        uint locationId = location->id();
        CanvasActiveInfo *info = getActiveUniform(program, locationId);
        int numValues = 4;

        switch (info->type()) {
        case SAMPLER_2D:
            // Intentional flow through
        case SAMPLER_CUBE:
            // Intentional flow through
        case INT: {
            GLint value = 0;
            glGetUniformiv(programId, locationId, &value);
            return QVariant::fromValue(value);
        }
        case INT_VEC2:
            numValues--;
            // Intentional flow through
        case INT_VEC3:
            numValues--;
            // Intentional flow through
        case INT_VEC4: {
            numValues--;
            GLint *value = new GLint[numValues];
            glGetUniformiv(programId, locationId, value);

            QList<float> intList;
            for (int i = 0; i < numValues; i++)
                intList << value[i];

            // TODO: Should return Int32Array
            return QVariant::fromValue(intList);
        }
        case FLOAT: {
            GLfloat value = 0;
            glGetUniformfv(programId, locationId, &value);
            return QVariant::fromValue(value);
        }
        case FLOAT_VEC2:
            numValues--;
            // Intentional flow through
        case FLOAT_VEC3:
            numValues--;
            // Intentional flow through
        case FLOAT_VEC4: {
            numValues--;
            GLfloat *value = new GLfloat[numValues];

            glGetUniformfv(programId, locationId, value);

            QList<float> floatList;
            for (int i = 0; i < numValues; i++)
                floatList << value[i];

            // TODO: Should return Float32Array
            return QVariant::fromValue(floatList);
        }
        case BOOL: {
            GLint value = 0;
            glGetUniformiv(programId, locationId, &value);
            return QVariant::fromValue(bool(value));
        }
        case BOOL_VEC2:
            numValues--;
            // Intentional flow through
        case BOOL_VEC3:
            numValues--;
            // Intentional flow through
        case BOOL_VEC4: {
            numValues--;
            GLint *value = new GLint[numValues];

            glGetUniformiv(programId, locationId, value);

            QList<bool> boolList;
            for (int i = 0; i < numValues; i++)
                boolList << value[i];

            return QVariant::fromValue(boolList);
        }
        case FLOAT_MAT2:
            numValues--;
            // Intentional flow through
        case FLOAT_MAT3:
            numValues--;
            // Intentional flow through
        case FLOAT_MAT4: {
            numValues = numValues * numValues;
            GLfloat *value = new GLfloat[numValues];

            glGetUniformfv(programId, locationId, value);

            QList<float> floatList;
            for (int i = 0; i < numValues; i++)
                floatList << value[i];

            // TODO: Should return Float32Array
            return QVariant::fromValue(floatList);
        }
        default:
            break;
        }
    }

    return QVariant();
}

/*!
 * \qmlmethod list<variant> Context3D::getSupportedExtensions()
 * Returns an array of the extension strings supported by this implementation
 */
/*!
 * \internal
 */
QVariantList CanvasContext::getSupportedExtensions()
{
    if (m_logAllCalls) qDebug() << Q_FUNC_INFO;

    // No extensions supported at the moment
    QVariantList list;
    list.append(QVariant::fromValue(QStringLiteral(QT_CANVAS3D_GL_STATE_DUMP_EXT_NAME)));
    return list;
}

/*!
 * \qmlmethod variant Context3D::getExtension(string name)
 * \return object if given \a name matches a supported extension.
 * Otherwise returns \c{null}. The returned object may contain constants and/or functions provided
 * by the extension, but at minimum a unique object is returned.
 * Case-insensitive \a name of the extension to be returned.
 */
/*!
 * \internal
 */
QVariant CanvasContext::getExtension(const QString &name)
{
    if (m_logAllCalls) qDebug() << "Context3D::" << __FUNCTION__
                                << "(name:" << name
                                << ")";

    QString upperCaseName = name.toUpper();

    if (upperCaseName == QStringLiteral(QT_CANVAS3D_GL_STATE_DUMP_EXT_NAME).toUpper()) {
        if (!m_stateDumpExt)
            m_stateDumpExt = new CanvasGLStateDump(m_context, this);
        return QVariant::fromValue(m_stateDumpExt);
    }

    return QVariant(QVariant::Int);
}
